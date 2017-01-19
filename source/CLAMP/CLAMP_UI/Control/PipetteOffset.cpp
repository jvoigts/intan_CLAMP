#include "PipetteOffset.h"
#include <QtGui>
#include "common.h"
#include "GUIUtil.h"
#include "GlobalState.h"
#include "Board.h"
#include "ClampController.h"
#include "DataAnalysis.h"
#include "Controller.h"
#include "ControlWindow.h"
#include "FeedbackBandwidthWidget.h"
#include <assert.h>

using std::vector;
using namespace CLAMP;
using namespace CLAMP::Registers;
using namespace CLAMP::ClampConfig;
using namespace CLAMP::SignalProcessing;

//--------------------------------------------------------------------------
PipetteOffsetWidget::PipetteOffsetWidget(GlobalState& state_, bool includeAutoButton, int unit_, Controller* controller_, FeedbackBandwidthWidget* feedback_) :
    QGroupBox(),
    autoButton(nullptr),
    state(state_),
    controller(controller_),
    feedback(feedback_),
	unit(unit_)
{
    pipetteOffsetSpinBox = GUIUtil::createDoubleSpinBox(255, 2.5, 0, " mV");
    pipetteOffsetSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    pipetteOffsetSpinBox->setStyleSheet("color:blue; background:transparent; border:none;");

//	QObject::connect(pipetteOffsetSpinBox, SIGNAL(valueChanged(double)), &state, SLOT(setPipetteOffset(double)));
    QObject::connect(pipetteOffsetSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setPipetteOffsetHere(double)));
//	QObject::connect(&state, SIGNAL(pipetteOffsetChanged(double)), pipetteOffsetSpinBox, SLOT(setValue(double)));
    QObject::connect(&state, SIGNAL(pipetteOffsetChanged(int, double)), this, SLOT(setValueHere(int, double)));

    QCheckBox* cb = GUIUtil::createLockBox(state.pipetteOffsetEnabled[unit_]);
    QObject::connect(&state.pipetteOffsetEnabled[unit_], SIGNAL(valueChanged(bool)), pipetteOffsetSpinBox, SLOT(setEnabled(bool)));
    pipetteOffsetSpinBox->setEnabled(state.pipetteOffsetEnabled[unit_].value());

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(pipetteOffsetSpinBox);
    layout->addStretch(1);
    layout->addWidget(cb);
    if (includeAutoButton) {
        assert(controller != nullptr);
        assert(feedback != nullptr);
        autoButton = new QPushButton("Auto", this);
        connect(autoButton, SIGNAL(clicked(bool)), this, SLOT(autoCalibrate()));
        QObject::connect(&state.pipetteOffsetEnabled[unit_], SIGNAL(valueChanged(bool)), this, SLOT(setAutoButtonEnable(bool)));
        autoButton->setEnabled(state.pipetteOffsetEnabled[unit_].value());
        layout->addStretch(1);
        layout->addWidget(autoButton);
    }

    setTitle(QObject::tr("Pipette Offset"));
    setLayout(layout);
}

void PipetteOffsetWidget::setAutoButtonEnable(bool value) {
    if (autoButton != nullptr) {
        if (state.isRunning()) {
            value = false;
        }
        autoButton->setEnabled(value);
    }
}

void PipetteOffsetWidget::autoCalibrate() {
    state.runThread(new PipetteOffsetThread(state, *controller, *feedback, unit));
}

void PipetteOffsetWidget::setPipetteOffsetHere(double value) {
	state.setPipetteOffset(unit, value);
}

void PipetteOffsetWidget::setValueHere(int unit_, double value) {
	if (unit_ == unit) {
		pipetteOffsetSpinBox->setValue(value);
	}
}

//--------------------------------------------------------------------------
PipetteOffsetThread::PipetteOffsetThread(GlobalState& state_, Controller& controller_, FeedbackBandwidthWidget& feedback_, int unit_) :
    state(state_),
    controller(controller_),
    feedback(feedback_),
	unit(unit_)
{
}

PipetteOffsetThread::~PipetteOffsetThread() {

}

void PipetteOffsetThread::done() {
    state.threadDone();
    state.pipetteOffsetEnabled[unit].setValue(false);
}

void PipetteOffsetThread::setRf(ChipChannelList& channelList, double currentRange) {
    Register3::Resistance rf;

	if (currentRange < 1.25e-9) {
		rf = Register3::Resistance::R80M;
	}
	else if (currentRange < 2.5e-9) {
        rf = Register3::Resistance::R40M;
    }
    else if (currentRange < 5e-9) {
        rf = Register3::Resistance::R20M;
    }
    else if (currentRange < 5e-8) {
        rf = Register3::Resistance::R2M;
    }
    else {
        rf = Register3::Resistance::R200k;
    }
    state.board->controller.currentToVoltageConverter.setFeedbackResistanceImmediate(channelList, rf);
}

double PipetteOffsetThread::measureCurrent(CLAMP::ClampConfig::ChipChannelList& channelList, int voltageSetting) {
    SimplifiedWaveform waveform;
    waveform.push_back(WaveformSegment(0, voltageSetting, state.board->getSamplingRateHz() * 0.2, 0, false, false));
    state.board->controller.simplifiedWaveformToWaveform(channelList, true, waveform);
	state.board->commandsToFPGASinglePort(unit);
    state.board->runAndReadOneCycle(unit);  // TODO: should this be 0 instead of unit?
    const vector<double>& results = state.board->readQueue.getMeasuredCurrents(channelList.front());
    double avg = DataAnalysis::calculateBestResidual(results.begin(), results.end() - 1);

    state.board->clearCommands();
    state.board->readQueue.clear();

    return avg;
}

void PipetteOffsetThread::run() {
    state.stateMessage(unit, "Auto-calibrating Pipette Offset");

    const int MAX_STEP = 10;  // corresponds to 25 mV

    int minSetting = -255, 
        setting = state.pipetteOffsetInmV[unit] / 2.5, 
        maxSetting = 255;
    double minValue = -std::numeric_limits<double>::infinity(),
        maxValue = std::numeric_limits<double>::infinity();

    // Get Rf value
    ChipChannelList channelList = { ChipChannel(unit, 0) };
    Register3::Resistance rf_orig = state.board->controller.currentToVoltageConverter.getFeedbackResistance(channelList.front());

    CapacitiveCompensationController& cap = *state.datastore[unit].controlWindow;
    state.board->controller.switchToVoltageClampImmediate(channelList, controller.getHoldingValue(), feedback.getDesiredBandwidth(), feedback.getResistanceEnum(), cap.getCapCompensationValue());

    while (keepGoing && ((maxSetting - minSetting) > 1))
    {
        // Set Rf based on minValue & maxValue
        setRf(channelList, maxValue - minValue);

        double value = measureCurrent(channelList, setting);

        if (value < 0) {
            minSetting = setting;
            minValue = value;

            int delta = std::min((maxSetting - minSetting)/2, MAX_STEP); // Go halfway there, or MAX_STEP at most
            delta = std::max(delta, 1);

            setting = minSetting + delta;
        }
        else {
            maxSetting = setting;
            maxValue = value;

            int delta = std::min((maxSetting - minSetting) / 2, MAX_STEP); // Go halfway there, or MAX_STEP at most
            delta = std::max(delta, 1);

            setting = maxSetting - delta;
        }
        state.setPipetteOffset(unit, setting * 2.5);
    }

    if (keepGoing) {
        if (std::abs(minValue) < std::abs(maxValue)) {
            setting = minSetting;
        }
        else {
            setting = maxSetting;
        }

        state.setPipetteOffset(unit, setting * 2.5);

        // Now go to the pipette offset setting
        measureCurrent(channelList, setting);
    }

    // Restore Rf value
    state.board->controller.currentToVoltageConverter.setFeedbackResistanceImmediate(channelList, rf_orig);

    // Go to holding value
    state.board->controller.clampVoltageGenerator.setClampVoltageImmediate(channelList, controller.getHoldingValue());
    controller.endMessage(unit);
}



