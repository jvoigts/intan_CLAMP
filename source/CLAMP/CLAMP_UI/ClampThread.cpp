#include "ClampThread.h"
#include "common.h"
#include "GlobalState.h"
#include "DisplayWindow.h"
#include "Plot.h"
#include "ResistanceWidget.h"
#include "FeedbackBandwidthWidget.h"
#include "common.h"
#include <QtGui>
#include <assert.h>
#include "Board.h"
#include "ControlWindow.h"
#include "WaveformAmplitudeWidget.h"

using std::vector;
using std::shared_ptr;
using std::exception;
using namespace CLAMP;
using namespace CLAMP::ClampConfig;
using namespace CLAMP::WaveformControl;

//------------------------------------------------------------------------------------------

ClampThread::ClampThread(GlobalState& state_, VoltageClampWidget** voltageWidget_, CurrentClampWidget** currentWidget_, bool* voltageClampMode_, unsigned int unit_) :
    Thread(),
    runType(ClampThread::CONTINUOUS),
    state(state_),
    board(*state_.board),
//	channelList({ ChipChannel(unit_, 0) }), /* TEMP - 200 kHz sampling */
//	channelList({ ChipChannel(unit_, 0), ChipChannel(unit_, 1) }), /* TEMP - 100 kHz sampling */
//	channelList({ ChipChannel(unit_, 0), ChipChannel(unit_, 1), ChipChannel(unit_, 2), ChipChannel(unit_, 3) }), /* TEMP - 50 kHz sampling */
//	channelList({ ChipChannel(unit_, 0), ChipChannel(unit_, 0), ChipChannel(unit_, 0), ChipChannel(unit_, 0) }), /* TEMP - 50 kHz sampling */
	voltageWidget(voltageWidget_),
	currentWidget(currentWidget_),
    capControlWidget(state.datastore[unit_].controlWindow),
	unit(unit_),
	capCompensationMagnitude(board.controller.fastTransientCapacitiveCompensation.getMagnitude(ChipChannel{ unit_, 0 })),
    capCompensationConnect(board.chip[unit_]->channel[0]->registers.r7.value.fastTransConnect)
{
	for (int i = 0; i < MAX_NUM_CHIPS; i++) {
		voltageClampMode[i] = voltageClampMode_[i];
	}
	if (voltageClampMode[unit_]) {
		controlWidget = voltageWidget[unit_];
	}
	else {
		controlWidget = currentWidget[unit_];
	}
	channelList = board.getPresentChannels();
	feedback = voltageWidget[unit_]->feedback;
	bandwidth = feedback->getDesiredBandwidth();
	resistance = feedback->getResistanceEnum();
}

ClampThread::~ClampThread() {
}

void ClampThread::done() {
    state.threadDone();
}

void ClampThread::run() {
	controlWidget->startMessage(unit);
	for (unsigned int i = 0; i < MAX_NUM_CHANNELS; i++) {
		if (i != unit) {
			if (voltageClampMode[i]) {
				voltageWidget[i]->endMessage(i);
			}
			else {
				currentWidget[i]->endMessage(i);
			}
		}
	}
    startRunning();
	unsigned int lastIndex = simplifiedWaveform[unit].lastIndex(state.datastore[unit].overlay);
	for (unsigned int i = 0; i < MAX_NUM_CHANNELS; i++) {
		if (state.board->chip[i]->present) {
			if (i == unit) {
				state.datastore[i].writeHeader(i);
			}
			else {
				state.datastore[i].writeHeader(i, true, lastIndex);
			}
		}
	}

    runWithType(runType);
    endRunning();
    controlWidget->endMessage(unit);
	for (unsigned int i = 0; i < MAX_NUM_CHANNELS; i++) {
		if (state.board->chip[i]->present) {
			state.datastore[i].closeFile();
		}
	}
}

void ClampThread::startRunning() {
	ChipChannelList chList;
	for (unsigned int i = 0; i < MAX_NUM_CHIPS; i++) {
		chList.push_back(ChipChannel{ i, 0 });
		if (voltageClampMode[i]) {
			switchToVoltageClamp({ ChipChannel{ i, 0 } }, voltageWidget[i]->getHoldingValue());
		}
		else {
			switchToCurrentClamp({ ChipChannel{ i, 0 } }, static_cast<CurrentScale>(controlWidget->getCurrentScale()), currentWidget[i]->getHoldingValue());
		}
	}
	board.controller.executeImmediate(chList);
	board.clearCommands();

	// Set other channels to holding
	for (unsigned int i = 0; i < MAX_NUM_CHIPS; i++) {
		if (i != unit) {
			if (voltageClampMode[i]) {
				board.controller.clampVoltageGenerator.setClampVoltage({ ChipChannel{ i, 0 } }, voltageWidget[i]->getHoldingValue());
			}
			else {
				board.controller.clampCurrentGenerator.setCurrent({ ChipChannel{ i, 0 } }, currentWidget[i]->getHoldingValue());
			}
		}
	}
	
	setCapacitiveCompensationImmediate();
	board.clearCommands();

	simplifiedWaveform[unit] = controlWidget->getSimplifiedWaveform(state.board->getSamplingRateHz());
	unsigned int lastIndex = simplifiedWaveform[unit].lastIndex(state.datastore[unit].overlay);
	for (int i = 0; i < MAX_NUM_CHIPS; i++) {
		if (i != unit) {
			if (voltageClampMode[i]) {
				simplifiedWaveform[i] = voltageWidget[i]->getSimplifiedWaveform(state.board->getSamplingRateHz(), i != unit, lastIndex);
				DEBUGOUT("simplifiedWaveform[i].waveform[0].appliedDiscreteValue = " << simplifiedWaveform[i].waveform[0].appliedDiscreteValue << endl);
			}
			else {
				simplifiedWaveform[i] = currentWidget[i]->getSimplifiedWaveform(state.board->getSamplingRateHz(), i != unit, lastIndex);
			}
		}
	}
	createWaveform();
}

void ClampThread::endRunning() {
	// Set to holding voltage or current
	for (unsigned int i = 0; i < MAX_NUM_CHIPS; i++) {
		if (voltageClampMode[i]) {
			for (int repeat = 0; repeat < 60; repeat++) {	// we must send multiple SPI commands so that the DACs update properly
				board.controller.clampVoltageGenerator.setClampVoltage({ ChipChannel{ i, 0 } }, voltageWidget[i]->getHoldingValue());
			}
		}
		else {
			for (int repeat = 0; repeat < 60; repeat++) {	// we must send multiple SPI commands so that the DACs update properly
				board.controller.clampCurrentGenerator.setCurrent({ ChipChannel{ i, 0 } }, currentWidget[i]->getHoldingValue());
			}
		}
	}
	board.controller.executeImmediate(channelList);
}

void ClampThread::clear(bool filtersToo) {
    board.readQueue.clear(filtersToo);
	if (!voltageClampMode[unit]) {
		clearOffsetVoltages();
	}
}

const vector<double>& ClampThread::getValues(unsigned int headstage) {
	if (voltageClampMode[headstage]) {
		return board.readQueue.getMeasuredCurrents({ ChipChannel{headstage, 0} });
	}
	else {
		const vector<double>& fromBoard = board.readQueue.getMeasuredVoltages(ChipChannel{ headstage, 0 });

		double offset = state.pipetteOffsetInmV[headstage] / 1000.0;
		if (offset == 0.0) {
			return fromBoard;
		}
		if (offset != voltageOffset) {
			clearOffsetVoltages();
			voltageOffset = offset;
		}
		if (offsetVoltages.size() != fromBoard.size()) {
			offsetVoltages.reserve(fromBoard.size());
			for (double v : fromBoard) {
				offsetVoltages.push_back(v - voltageOffset);
			}
		}
		return offsetVoltages;
	}
}

const vector<double>& ClampThread::getClampValues(unsigned int headstage) {
	if (voltageClampMode[headstage]) {
		return board.readQueue.getClampVoltages(ChipChannel{ headstage, 0 });
	}
	else {
		return board.readQueue.getClampCurrents(ChipChannel{ headstage, 0 });
	}
}

void ClampThread::clearOffsetVoltages() {
	offsetVoltages.erase(offsetVoltages.begin(), offsetVoltages.end());
}

void ClampThread::switchToCurrentClamp(const ChipChannel& channel, CurrentScale scale, int holdingCurrent) {
	// board.controller.switchToCurrentClampImmediate(channelList, scale, holdingCurrent, capControlWidget->getCapCompensationValue());
	board.controller.switchToCurrentClamp(channel, scale, holdingCurrent, capControlWidget->getCapCompensationValue());
}

void ClampThread::switchToVoltageClamp(const ChipChannel& channel, int holdingVoltage) {
	// board.controller.switchToVoltageClampImmediate(channelList, holdingVoltage, feedback->getDesiredBandwidth(), feedback->getResistanceEnum(), capControlWidget->getCapCompensationValue());
	board.controller.switchToVoltageClamp(channel, holdingVoltage, feedback->getDesiredBandwidth(), feedback->getResistanceEnum(), capControlWidget->getCapCompensationValue());
}

void ClampThread::setRCImmediate() {
	// Set the feedback resistor and capacitor values
	bandwidth = feedback->getDesiredBandwidth();
	board.chip[unit]->channel[0]->desiredBandwidth = bandwidth;
	resistance = feedback->getResistanceEnum();
	board.controller.currentToVoltageConverter.setFeedbackResistanceAndCapacitanceImmediate({ ChipChannel{ unit, 0 } }, resistance);
}

void ClampThread::setScaleImmediate() {
	if (voltageClampMode[unit]) {
		setRCImmediate();
	}
}

bool ClampThread::scaleChanged() const {
	return (bandwidth != feedback->getDesiredBandwidth()) || (resistance != feedback->getResistanceEnum());
}

void ClampThread::setCapacitiveCompensationImmediate() {
    capCompensationMagnitude = board.controller.fastTransientCapacitiveCompensation.getMagnitude({ ChipChannel{ unit, 0 } });
    board.controller.fastTransientCapacitiveCompensation.setMagnitudeImmediate({ ChipChannel{ unit, 0 } }, capCompensationMagnitude);
    capCompensationConnect = board.chip[unit]->channel[0]->registers.r7.value.fastTransConnect;
    board.controller.fastTransientCapacitiveCompensation.setConnectImmediate({ ChipChannel{ unit, 0 } }, capCompensationConnect);
}

bool ClampThread::capCompensationChanged() const {
    return (capCompensationMagnitude != board.controller.fastTransientCapacitiveCompensation.getMagnitude({ ChipChannel{ unit, 0 } })) ||
           (capCompensationConnect != board.chip[unit]->channel[0]->registers.r7.value.fastTransConnect);
}

void ClampThread::createWaveform() {
    board.enableChannels(channelList, true);
	// board.addEnabledChannels(channelList);

	for (unsigned int i = 0; i < MAX_NUM_CHIPS; i++) {
		board.controller.simplifiedWaveformToWaveform({ ChipChannel{ i, 0 } }, voltageClampMode[i], simplifiedWaveform[i]);
	}
    board.commandsToFPGA();

    vector<uint16_t> digitalOutputs;
    digitalOutputs.reserve(simplifiedWaveform[unit].size());
    for (unsigned int i = 0; i < simplifiedWaveform[unit].size(); i++) {
        for (unsigned int j = 0; j < simplifiedWaveform[unit].waveform[i].numCommands; j++) {
			digitalOutputs.push_back((simplifiedWaveform[unit].waveform[i].appliedDiscreteValue == simplifiedWaveform[unit].waveform[0].appliedDiscreteValue) ? 
				0 : (board.digitalMarkerEnabled(unit) << board.digitalMarkerDestination(unit)));
        }
    }
}

void ClampThread::readAndProcessOneCycle(bool first, double time) {
    unsigned int packetsToRead = board.getNumTimesteps(unit);
    if (first) {
        packetsToRead++;
    }
    while (keepGoing && packetsToRead > 0) {
        unsigned int packetsThisRead = board.read(packetsToRead);

		packetsToRead -= packetsThisRead;
		for (auto& index : channelList) {
			state.datastore[index.chip].storeData(getValues(index.chip), getClampValues(index.chip), time);
		}
        clear(false);

        state.datastore[unit].controlWindow->updateStatsExt();
    }
}

void ClampThread::finishLastCycle() {
    board.stop();
    board.flush();

    board.readQueue.pushLast();
    clear(true);
}

void ClampThread::runWithType(RunType runType) {
    switch (runType)
    {
    case ClampThread::BATCH:
        if (simplifiedWaveform[unit].interval > 0) {
            // There's no way to run continuously if you're pausing between runs
            runBatches();
        }
        else {
            runSemicontinuously();
        }
        break;
    case ClampThread::CONTINUOUS:
        if (simplifiedWaveform[unit].interval > 0) {
            // There's no way to run continuously if you're pausing between runs
            runBatches();
        }
        else {
            runContinuously();
        }
        break;
    case ClampThread::ONCE:
        runOnce();
        break;
    default:
        break;
    }
}

void ClampThread::runOnce() {
    double time = 0;
    try {
        board.runOneCycle(unit, 1);
		for (auto& index : channelList) {
			state.datastore[index.chip].init(simplifiedWaveform[index.chip], voltageClampMode[index.chip]);
		}
		for (auto& index : channelList) {
			state.datastore[index.chip].startCycle();
		}
        readAndProcessOneCycle(true, time);
        finishLastCycle();
    }
    catch (exception&) {
        board.flush();
    }
    board.clearCommands();
}

void ClampThread::runContinuously() {
    board.runContinuously();

	for (auto& index : channelList) {
		state.datastore[index.chip].init(simplifiedWaveform[index.chip], voltageClampMode[index.chip]);
	}
    bool first = true;
    while (keepGoing) {
        // Now execute
		for (auto& index : channelList) {
			state.datastore[index.chip].startCycle();
		}
        readAndProcessOneCycle(first);
        first = false;
    }
    finishLastCycle();

    board.clearCommands();
}

void ClampThread::runSemicontinuously() {
    board.runContinuously();
	for (auto& index : channelList) {
		state.datastore[index.chip].init(simplifiedWaveform[index.chip], voltageClampMode[index.chip]);
	}
    bool first = true;
    while (keepGoing) {
        controlWidget->startMessage(unit);

        SimplifiedWaveform newWaveform = controlWidget->getSimplifiedWaveform(state.board->getSamplingRateHz());
        bool different = (simplifiedWaveform[unit] != newWaveform) || scaleChanged() || capCompensationChanged();
        if (different) {
            finishLastCycle();
            //board.clearCommands();
			board.clearSelectedCommands(channelList);
            setScaleImmediate();
            setCapacitiveCompensationImmediate();

            simplifiedWaveform[unit] = newWaveform;
            createWaveform();
			for (auto& index : channelList) {
				state.datastore[index.chip].init(simplifiedWaveform[index.chip], voltageClampMode[index.chip]);
			}
            board.runContinuously();
        }
        try {
			for (auto& index : channelList) {
				state.datastore[index.chip].startCycle();
			}
            readAndProcessOneCycle(first);
            first = false;
        }
        catch (exception& e) {
            state.errorMessage("Error - not enough data", e.what());
            board.flush();
        }
    }
    finishLastCycle();
    board.clearCommands();
}

void ClampThread::runBatches() {
    double time = 0;
	for (auto& index : channelList) {
		state.datastore[index.chip].init(simplifiedWaveform[index.chip], voltageClampMode[index.chip]);
	}
    while (keepGoing) {
        controlWidget->startMessage(unit);
        // Do these things each time through
        //board.clearCommands();
		board.clearSelectedCommands(channelList);
        setScaleImmediate();
        setCapacitiveCompensationImmediate();

        SimplifiedWaveform newWaveform = controlWidget->getSimplifiedWaveform(state.board->getSamplingRateHz());
        bool different = simplifiedWaveform[unit] != newWaveform;
        simplifiedWaveform[unit] = newWaveform;
        createWaveform();
        if (different) {
			for (auto& index : channelList) {
				state.datastore[index.chip].init(simplifiedWaveform[index.chip], voltageClampMode[index.chip]);
			}
        }
        try {
            board.runOneCycle(unit, 1);
			for (auto& index : channelList) {
				state.datastore[index.chip].startCycle();
			}
            readAndProcessOneCycle(true, time);
            finishLastCycle();
        }
        catch (exception& e) {
            state.errorMessage("Error - not enough data", e.what());
            board.flush();
        }

        double intervalS = simplifiedWaveform[unit].interval;
        if (intervalS > 0) {
            double readTimeMs = (state.board->getNumTimesteps(unit)) / state.board->getSamplingRateHz() * 1000;
            double intervalMs = intervalS * 1000;
            int sleepTime = intervalMs - readTimeMs;
            time += (sleepTime > 0) ? intervalS : 1;
            while (keepGoing && sleepTime > 0) {
                int thisRound = std::min(sleepTime, 1000);
                std::this_thread::sleep_for(std::chrono::milliseconds(thisRound));
                sleepTime -= thisRound;
            }
        }
    }
    board.clearCommands();
}
