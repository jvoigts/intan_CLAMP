#include "DataStore.h"
#include "Plot.h"
#include "common.h"
#include "DisplayWindow.h"
#include "DataAnalysis.h"
#include "BesselFilter.h"
#include "Board.h"
#include "ControlWindow.h"
#include "SaveFile.h"
#include <exception>
#include <assert.h>
#include "Line.h"
#include "VoltageClampWidget.h"
#include <cmath>
#include "qstring.h"
#include "qdatetime.h"
#include "qfileinfo.h"

using namespace CLAMP;
using namespace CLAMP::SignalProcessing;
using namespace CLAMP::IO;
using namespace CLAMP::ClampConfig;
using std::vector;
using std::wstring;
using std::invalid_argument;
using std::lock_guard;
using std::recursive_mutex;
using std::isnormal;
using std::isfinite;

//--------------------------------------------------------------------------
DataProcessor::DataProcessor(DataStore& datastore_) :
    datastore(datastore_)
{
}

//--------------------------------------------------------------------------
AppliedWaveformProcessor::AppliedWaveformProcessor(DataStore& datastore_) :
    DataProcessor(datastore_)
{
    waveforms.tStep = 1.0 / datastore.state->board->getSamplingRateHz();
}

AppliedWaveformProcessor::~AppliedWaveformProcessor() {
}

void AppliedWaveformProcessor::init() {
    createAppliedWaveformPlot();
}

void AppliedWaveformProcessor::reset() {
    createAppliedWaveformPlot();
}

void AppliedWaveformProcessor::process(bool overlayChanged, bool) {
    if (overlayChanged) {
        createAppliedWaveformPlot();
    }
}

void AppliedWaveformProcessor::getAppliedWaveforms(double samplingRate) {
    appliedWaveforms.erase(appliedWaveforms.begin(), appliedWaveforms.end());
    appliedWaveforms.resize(datastore.simplifiedWaveform.numWaveforms());

	for (unsigned int i = 0; i < datastore.simplifiedWaveform.size(); i++) {
        const WaveformSegment& rs = datastore.simplifiedWaveform.waveform[i];
        Line& w = appliedWaveforms[rs.waveformNumber];
        if (i > 0 && (datastore.simplifiedWaveform.waveform[i - 1].waveformNumber != rs.waveformNumber)) {
            w.addLineSegment();
        }

        double value = datastore.simplifiedWaveform.waveform[i].appliedValue;

        unsigned int startIndex = rs.startIndex;
        unsigned int endIndex = rs.endIndex;
        if (datastore.overlay) {
            startIndex -= rs.tOffset;
            endIndex -= rs.tOffset;
        }

        w.addPoint(startIndex / samplingRate, value);
        w.addPoint(endIndex / samplingRate, value);
    }
}

void AppliedWaveformProcessor::createAppliedWaveformPlot() {
    getAppliedWaveforms(datastore.state->board->getSamplingRateHz());
    waveforms.setLines(appliedWaveforms);
}

//--------------------------------------------------------------------------
LineIncrements seriesResistanceCorrect(DataStore& datastore, const std::vector<double>& values, double samplingRate, bool doCorrection, correctFunction f) {
    LineIncrements results;
    results.startIndices.resize(datastore.simplifiedWaveform.numWaveforms());
    results.lines.resize(datastore.simplifiedWaveform.numWaveforms());
    for (unsigned int i = 0; i < datastore.simplifiedWaveform.size(); i++) {
        const WaveformSegment& segment = datastore.simplifiedWaveform.waveform[i];
        if (segment.endIndex >= datastore.startAt && values.size() >= segment.startIndex) {
            Line& w = results.lines[segment.waveformNumber];
            if (w.data.empty()) {
                results.startIndices[segment.waveformNumber] = segment.indexWithinWaveform;
            }
            if (i > 0 && (datastore.simplifiedWaveform.waveform[i - 1].waveformNumber != segment.waveformNumber)) {
                w.addLineSegment();
            }
            for (unsigned int j = segment.startIndex; j < values.size() && j <= segment.endIndex; j++) {
                if (j >= datastore.startAt) {
                    double value = values[j];
                    if (!std::isnan(value)) {
                        unsigned int j2 = datastore.overlay ? j - segment.tOffset : j;
                        double time = datastore.timestamps[j2] / samplingRate;
                        time -= datastore.cycleStartTime;

                        w.addPoint(time, (*f)(doCorrection, segment.appliedValue, value, datastore.Ra));
                    }
                }
            }
        }
    }
    return results;
}

//--------------------------------------------------------------------------
AppliedPlusAdcProcessor::AppliedPlusAdcProcessor(DataStore& datastore_, AppliedWaveformProcessor& applied_, std::vector<double>& clampValues_) :
	DataProcessor(datastore_),
	applied(applied_),
	clampValues(clampValues_)
{
}

AppliedPlusAdcProcessor::~AppliedPlusAdcProcessor() {
}

double AppliedPlusAdcProcessor::dummyFunction(bool correct, double applied, double value, double r) {  // TODO: Dummy function is cheesy.
	return value;
}

void AppliedPlusAdcProcessor::process(bool overlayChanged, bool dataChanged) {
	if (dataChanged || overlayChanged) {
		double samplingRate = datastore.state->board->getSamplingRateHz();
		applied.waveforms.appendLines(datastore.simplifiedWaveform.numWaveforms(), seriesResistanceCorrect(datastore, clampValues, samplingRate, false, dummyFunction));
	}
}

//--------------------------------------------------------------------------
VCellProcessor::VCellProcessor(DataStore& datastore_, AppliedWaveformProcessor& applied_, FilterProcessor& source_) :
    DataProcessor(datastore_),
    applied(applied_),
    source(source_)
{
}

VCellProcessor::~VCellProcessor() {
}

double VCellProcessor::vCellCorrect(bool correct, double applied, double value, double r) {
    // Applied is voltage, value is current
    if (correct) {
        return applied - value * r;
    }
    else {
        return applied;
    }
}

void VCellProcessor::process(bool overlayChanged, bool dataChanged) {
    if (dataChanged || overlayChanged) {
        double samplingRate = datastore.state->board->getSamplingRateHz();
        const vector<double>& values = source.getValues();
        applied.waveforms.appendLines(datastore.simplifiedWaveform.numWaveforms(), seriesResistanceCorrect(datastore, values, samplingRate, true, vCellCorrect));
    }
}

//--------------------------------------------------------------------------
MeasuredWaveformProcessor::MeasuredWaveformProcessor(DataStore& datastore_, FilterProcessor& filter, bool bridgeBalance_) :
    DataProcessor(datastore_),
    source(filter),
    bridgeBalance(bridgeBalance_)
{
    waveforms.tStep = 1.0 / datastore.state->board->getSamplingRateHz();
}

MeasuredWaveformProcessor::~MeasuredWaveformProcessor() {
}

void MeasuredWaveformProcessor::init() {
    waveforms.clearLines();
}

void MeasuredWaveformProcessor::reset() {
    waveforms.cycleLines();
}

void MeasuredWaveformProcessor::process(bool overlayChanged, bool dataChanged) {
    if (overlayChanged || dataChanged) {
        LineIncrements measuredWaveforms = getMeasuredWaveforms(source.getValues(), datastore.state->board->getSamplingRateHz());
        waveforms.appendLines(0, measuredWaveforms);
    }
}

double MeasuredWaveformProcessor::bridgeBalanceCorrect(bool correct, double applied, double value, double r) {
    // Applied is current, value is voltage
    if (correct) {
        return value - r * applied;
    }
    else {
        return value;
    }
}

LineIncrements MeasuredWaveformProcessor::getMeasuredWaveforms(const vector<double>& values, double samplingRate) {
    return seriesResistanceCorrect(datastore, values, samplingRate, bridgeBalance, bridgeBalanceCorrect);
}

//--------------------------------------------------------------------------
DCCalculationProcessor::DCCalculationProcessor(DataStore& datastore_, FilterProcessor& filter) :
    DataProcessor(datastore_),
    source(filter)
{
}

DCCalculationProcessor::~DCCalculationProcessor() {
}

void DCCalculationProcessor::init() {
    waveformCalculations.clear();
    waveformCalculations.resize(datastore.simplifiedWaveform.size());
}

void DCCalculationProcessor::reset() {
    waveformCalculations.clear();
    waveformCalculations.resize(datastore.simplifiedWaveform.size());
}

void DCCalculationProcessor::process(bool, bool dataChanged) {
    if (dataChanged) {
        getSteadyStateValues(source.getValues());
    }
}

void DCCalculationProcessor::getSteadyStateValues(const vector<double>& values) {
    for (unsigned int i = 0; i < datastore.simplifiedWaveform.size(); i++) {
        const WaveformSegment& element = datastore.simplifiedWaveform.waveform[i];
        bool needsCalculation = datastore.dataAvailable(i) && !waveformCalculations[i].valid;
        bool nontrivial = datastore.simplifiedWaveform.waveform[i].numReps() > 1;
        if (needsCalculation && nontrivial) {
            waveformCalculations[i].steadyStateValue = DataAnalysis::calculateBestResidual(values.begin() + element.startIndex, values.begin() + element.endIndex);
            waveformCalculations[i].valid = true;
        }
    }
}

//--------------------------------------------------------------------------
DCPlotProcessor::DCPlotProcessor(DataStore& datastore_, Lines& waveforms_, DCCalculationProcessor& calc) :
    DataProcessor(datastore_),
    waveforms(waveforms_),
    dcCalculation(calc)
{
}

DCPlotProcessor::~DCPlotProcessor() {
}

void DCPlotProcessor::init() {
}

void DCPlotProcessor::reset() {
}

void DCPlotProcessor::process(bool overlayChanged, bool dataChanged) {
    if (overlayChanged || dataChanged) {
        LineIncrements residualWaveforms = getResidualWaveforms(datastore.state->board->getSamplingRateHz());
        waveforms.appendLines(datastore.simplifiedWaveform.numWaveforms(), residualWaveforms);
    }
}

LineIncrements DCPlotProcessor::getResidualWaveforms(double samplingRate) {
    LineIncrements results;
    results.startIndices.resize(datastore.simplifiedWaveform.numWaveforms());
    results.lines.resize(datastore.simplifiedWaveform.numWaveforms());
    vector<Line>& residualWaveforms = results.lines;

    // Now we'll add waveforms for the residuals
    for (unsigned int i = 0; i < datastore.simplifiedWaveform.size(); i++) {
        const WaveformSegment& element = datastore.simplifiedWaveform.waveform[i];
        bool nontrivial = datastore.simplifiedWaveform.waveform[i].numReps() > 1;
        if ((datastore.startAt <= element.endIndex) && datastore.dataAvailable(i) && nontrivial) {
            Line& w = residualWaveforms[element.waveformNumber];

            // Always want to start a new piece.  If there are none, create a blank one to start with; otherwise add a new one
            if (w.data.empty()) {
                results.startIndices[element.waveformNumber] = element.indexWithinWaveform;
            }
            w.addLineSegment();

            double value = dcCalculation.waveformCalculations[i].steadyStateValue;
            unsigned int startIndex = element.startIndex + element.numReps() / 2;
            unsigned int endIndex = element.endIndex;
            if (datastore.overlay) {
                startIndex -= element.tOffset;
                endIndex -= element.tOffset;
            }

            w.addPoint(startIndex / samplingRate, value);
            w.addPoint(endIndex / samplingRate, value);
        }
    }
    return results;
}

//--------------------------------------------------------------------------
ExponentialCalculationWaveformProcessor::ExponentialCalculationWaveformProcessor(DataStore& datastore_, FilterProcessor& filter) :
    DataProcessor(datastore_),
    source(filter)
{
}

ExponentialCalculationWaveformProcessor::~ExponentialCalculationWaveformProcessor() {
}

void ExponentialCalculationWaveformProcessor::init() {
    exponentialParameters.clear();
    exponentialParameters.resize(datastore.simplifiedWaveform.size());
}

void ExponentialCalculationWaveformProcessor::reset() {
    exponentialParameters.clear();
    exponentialParameters.resize(datastore.simplifiedWaveform.size());
}

void ExponentialCalculationWaveformProcessor::process(bool, bool dataChanged) {
    if (dataChanged) {
        fitExponentials(source.getValues(), datastore.state->board->getSamplingRateHz());
    }
}

void ExponentialCalculationWaveformProcessor::fitExponentials(const vector<double>& values, double samplingRate) {
    for (unsigned int i = 0; i < datastore.simplifiedWaveform.size(); i++) {
        bool needsCalculation = datastore.dataAvailable(i) && !exponentialParameters[i].valid;
        bool hasTransient = i > 0
            && datastore.simplifiedWaveform.waveform[i].numReps() > 1
            && (datastore.simplifiedWaveform.waveform[i].appliedDiscreteValue != datastore.simplifiedWaveform.waveform[i - 1].appliedDiscreteValue); // use simplifiedWaveform because it stores ints, not doubles
        if (needsCalculation && hasTransient) {
            const WaveformSegment& element = datastore.simplifiedWaveform.waveform[i];

            vector<double> xs, ys;
            unsigned int startIndex = element.startIndex;
            unsigned int endIndex = element.endIndex;
            if (isnan(values[endIndex])) {
                endIndex--;
            }

            startIndex += 12;

            double t0 = datastore.timestamps[element.startIndex] / samplingRate;
            for (unsigned int j = startIndex; j <= endIndex; j++) {
                double y = values[j];
                double t = datastore.timestamps[j] / samplingRate;
                xs.push_back(t - t0);
                ys.push_back(y);
            }
            double chi2;
            ExponentialFit::lm(xs, ys, exponentialParameters[i].beta, chi2);

            exponentialParameters[i].valid = true;
        }
    }
}

//--------------------------------------------------------------------------
ExponentialPlotProcessor::ExponentialPlotProcessor(DataStore& datastore_, Lines& waveforms_, ExponentialCalculationWaveformProcessor& calc) :
    DataProcessor(datastore_),
    waveforms(waveforms_),
    calculator(calc)
{
}

ExponentialPlotProcessor::~ExponentialPlotProcessor() {
}

void ExponentialPlotProcessor::init() {

}

void ExponentialPlotProcessor::reset() {

}

void ExponentialPlotProcessor::process(bool overlayChanged, bool dataChanged) {
    if (overlayChanged || dataChanged) {
        LineIncrements exponentials = getExponentialWaveforms(datastore.state->board->getSamplingRateHz());
        waveforms.appendLines(2 * datastore.simplifiedWaveform.numWaveforms(), exponentials);
    }
}

LineIncrements ExponentialPlotProcessor::getExponentialWaveforms(double samplingRate) {
    LineIncrements results;
    results.startIndices.resize(datastore.simplifiedWaveform.numWaveforms());
    results.lines.resize(datastore.simplifiedWaveform.numWaveforms());
    vector<Line>& exponentials = results.lines;

    // Add waveform(s) for exponentials
    for (unsigned int i = 1; i < datastore.simplifiedWaveform.size(); i++) {
        const WaveformSegment& element = datastore.simplifiedWaveform.waveform[i];
        if ((datastore.startAt <= element.endIndex) && calculator.exponentialParameters[i].valid) {
            Line& w = exponentials[element.waveformNumber];

            // Always want to start a new piece.  If there are none, create a blank one to start with; otherwise add a new one
            if (w.data.empty()) {
                results.startIndices[element.waveformNumber] = element.indexWithinWaveform;
            }
            w.addLineSegment();

            double t0 = datastore.timestamps[element.startIndex] / samplingRate;
            double tElementOffset = datastore.timestamps[element.tOffset] / samplingRate;
            for (unsigned int j = element.startIndex; j <= element.endIndex; j++) {
                double t = datastore.timestamps[j] / samplingRate;
                double tCorrected = t - t0;
                double value = ExponentialFit::f(tCorrected, calculator.exponentialParameters[i].beta);

                if (datastore.overlay) {
                    t -= tElementOffset;
                }
                else {
                    t -= datastore.cycleStartTime;
                }
                w.addPoint(t, value);
            }
        }
    }
    return results;
}

//--------------------------------------------------------------------------
ResistanceCalculationWaveformProcessor::ResistanceCalculationWaveformProcessor(DataStore& datastore_, DCCalculationProcessor* dc_, ExponentialCalculationWaveformProcessor* exp_) :
    DataProcessor(datastore_),
    dc(dc_),
    exp(exp_)
{
    if (dc == nullptr && exp == nullptr) {
        throw invalid_argument("You need at least one source of resistance values");
    }
}

ResistanceCalculationWaveformProcessor::~ResistanceCalculationWaveformProcessor() {
}

void ResistanceCalculationWaveformProcessor::process(bool, bool dataChanged) {
    if (dataChanged) {
        vector<double> applied, measured;
        for (unsigned int i = 0; i < datastore.simplifiedWaveform.size(); i++) {
            bool nontrivial = datastore.simplifiedWaveform.waveform[i].numReps() > 1;
            if (nontrivial) {
                bool hasValue = false;
                double measuredValue;
                if (exp && exp->exponentialParameters[i].valid) {
                    measuredValue = exp->exponentialParameters[i].beta[0];
                    hasValue = true;
                }
                else if (dc && dc->waveformCalculations[i].valid) {
                    measuredValue = dc->waveformCalculations[i].steadyStateValue;
                    hasValue = true;
                }
                if (hasValue) {
                    applied.push_back(datastore.simplifiedWaveform.waveform[i].appliedValue);
                    measured.push_back(measuredValue);
                }
            }
        }

        if (measured.size() >= 2) {
            double resistance;
            if (datastore.applyVoltages) {
                resistance = calculateR(measured, applied);
            }
            else {
                resistance = calculateR(applied, measured);
            }
            if (isnormal(resistance) && resistance > 0) {
                datastore.resistance = resistance;
                datastore.controlWindow->setResistance(resistance);
            }
        }
    }

}

double ResistanceCalculationWaveformProcessor::calculateR(const std::vector<double>& currents, const std::vector<double>& voltages) {
    return DataAnalysis::slope(currents.begin(), currents.end(), voltages.begin(), voltages.end());
}

//--------------------------------------------------------------------------
CellParameterProcessor::CellParameterProcessor(DataStore& datastore_, ExponentialCalculationWaveformProcessor& exp_) :
    DataProcessor(datastore_),
    exp(exp_)
{
}

CellParameterProcessor::~CellParameterProcessor() {
}

void CellParameterProcessor::getRsAndCs(double beta[3], double dV, double resistance, double& Ra, double& Rm, double& Cm) {
    //double A = beta[0];
    double B = beta[1];
    double C = beta[2];
    double X = resistance;
    double Y = C / dV;
    double tau = -1.0 / B;
    Ra = X / (1.0 + X * Y);
    Rm = X - Ra;
    double RaPRm = 1.0 / (1.0 / Ra + 1.0 / Rm);
    Cm = tau / RaPRm;
}

void CellParameterProcessor::calculateCellParameters(double& Ra, double& Rm, double& Cm) {
    vector<double> Ras, Rms, Cms;
    for (unsigned int i = 0; i < datastore.simplifiedWaveform.size(); i++) {
        if (exp.exponentialParameters[i].valid) {
            double Ra, Rm, Cm;
            double dV = datastore.simplifiedWaveform.waveform[i].appliedValue - datastore.simplifiedWaveform.waveform[i - 1].appliedValue;
            getRsAndCs(exp.exponentialParameters[i].beta, dV, datastore.resistance, Ra, Rm, Cm);
            Ras.push_back(Ra);
            Rms.push_back(Rm);
            Cms.push_back(Cm);
        }
    }

    Ra = DataAnalysis::average(Ras.begin(), Ras.end());
    Rm = DataAnalysis::average(Rms.begin(), Rms.end());
    Cm = DataAnalysis::average(Cms.begin(), Cms.end());
}

void CellParameterProcessor::process(bool, bool dataChanged) {
    if (dataChanged) {
        double Ra, Rm, Cm;
        calculateCellParameters(Ra, Rm, Cm);
        if (isfinite(Ra) && isfinite(Rm) && isfinite(Cm)) {
            datastore.setWholeCell(Ra, Rm, Cm);
        }
    }
}


//--------------------------------------------------------------------------
ResistanceProcessor::ResistanceProcessor(DataStore& datastore_) :
    DataProcessor(datastore_)
{
    waveform.tStep = 1.0;
}

ResistanceProcessor::~ResistanceProcessor() {
}

void ResistanceProcessor::process(bool, bool dataChanged) {
    // This is true at the end of a cycle
    if (dataChanged && !datastore.simplifiedWaveform.waveform.empty() && (datastore.rawValues.size() > datastore.simplifiedWaveform.waveform.back().endIndex)) {
        waveform.addToLine(0, datastore.absoluteTime, datastore.resistance);
    }
}

//--------------------------------------------------------------------------
FilterProcessor::FilterProcessor(DataStore& datastore_, std::vector<double>& rawValues_) :
    DataProcessor(datastore_),
    filter(new NthOrderBesselLowPassFilter(4, 10800.0, 1 / datastore_.state->board->getSamplingRateHz())),
    lowPassFilterEnabled(false),
    rawValues(rawValues_)
{
    connect(&datastore, SIGNAL(lowPassFilterEnabledChanged(bool)), this, SLOT(enableLowPassFilter(bool)));
    connect(&datastore, SIGNAL(lowPassFilterCutoffChanged(double)), this, SLOT(setLowPassFilterCutoff(double)));
}

FilterProcessor::~FilterProcessor() {
}

void FilterProcessor::init() {
    filter->reset();
    filteredValues.erase(filteredValues.begin(), filteredValues.end());
}

void FilterProcessor::reset() {
    filter->reset();
    filteredValues.erase(filteredValues.begin(), filteredValues.end());
}

void FilterProcessor::process(bool, bool dataChanged) {
    if (dataChanged) {
        filteredValues.resize(rawValues.size());
        for (unsigned int i = datastore.startAt; i < rawValues.size(); i++) {
            filteredValues[i] = filter->filterOne(rawValues[i]);
        }
    }
}

void FilterProcessor::enableLowPassFilter(bool enable) {
    lowPassFilterEnabled = enable;
}

void FilterProcessor::setLowPassFilterCutoff(double fc) {
    filter.reset(new NthOrderBesselLowPassFilter(4, fc, 1 / datastore.state->board->getSamplingRateHz()));
}

const vector<double>& FilterProcessor::getValues() {
    return lowPassFilterEnabled ? filteredValues : rawValues;
}
//--------------------------------------------------------------------------

// Note: User must set 'state' pointer before use.  We don't to it here because we must be able to declare an array of DataStore objects,
// which requires a default contructor.
DataStore::DataStore() :
	resistance(0),
	displayWindow(nullptr),
	controlWindow(nullptr),
	cycleStartTime(0.0),
	absoluteTime(-1),
	cellParametersValue(false),
	Ra(0), // was -1
	Rm(0),
	Cm(0),
	saveFile(nullptr),
	saveFileAux(nullptr)
{
	lock_guard<recursive_mutex> lock(datastoreMutex);

	adcs.resize(8);
	adcsDouble.resize(8);

	numAdcs = 8;
}

/*
DataStore::DataStore() :
...
*/



DataStore::~DataStore() {
    closeFile();
}

void DataStore::openFile(const QString& subdirName, const QString& baseFilename, const QDateTime& dateTime, int unit, bool auxDataToo) {
	QFileInfo fileInfo(baseFilename);
	QFileInfo subdirInfo(subdirName);

	QString unitDesignator;
	switch (unit) {
	case 0:
		unitDesignator = "A";
		break;
	case 1:
		unitDesignator = "B";
		break;
	case 2:
		unitDesignator = "C";
		break;
	case 3:
		unitDesignator = "D";
		break;
	case 4:
		unitDesignator = "E";
		break;
	case 5:
		unitDesignator = "F";
		break;
	case 6:
		unitDesignator = "G";
		break;
	case 7:
		unitDesignator = "H";
		break;
	default:
		unitDesignator = "X";
	}

	QString filename = fileInfo.path() +  "/" + subdirInfo.baseName() + "/" + fileInfo.baseName() + "_" + unitDesignator +
		"_" +dateTime.toString("yyMMdd") + "_" + dateTime.toString("HHmmss") + ".clp";

		saveFile = new SaveFile();
		saveFile->open(toFileName(filename.toStdString()));

	if (auxDataToo) {
		QString filenameAux = fileInfo.path() + "/" + subdirInfo.baseName() + "/" + fileInfo.baseName() + "_" + "AUX" +
			"_" + dateTime.toString("yyMMdd") + "_" + dateTime.toString("HHmmss") + ".clp";

		saveFileAux = new SaveFile();
		saveFileAux->open(toFileName(filenameAux.toStdString()));
	}

	numAdcs = state->board->expanderBoardPresent() ? 8 : 2;
}

void DataStore::writeHeader(int unit, bool holdingOnly, unsigned int lastIndex) {
    if (saveFile) {
        // Do this after start, so that we save settings as they are during run
        HeaderData header(*state->board, ChipChannel(unit, 0));
        fillSaveHeader(header, unit, holdingOnly, lastIndex);

        saveFile->writeHeader(header);
    }
	if (saveFileAux) {
		AuxHeaderData auxHeader(*state->board, ChipChannel(unit, 0), numAdcs);
		saveFileAux->writeHeaderAux(auxHeader);
	}
}

void DataStore::closeFile() {
    if (saveFile) {
        saveFile->close();
        delete saveFile;
        saveFile = nullptr;
    }
	if (saveFileAux) {
		saveFileAux->close();
		delete saveFileAux;
		saveFileAux = nullptr;
	}
}

void DataStore::writeToFile() {
    if (saveFile) {
        lock_guard<recursive_mutex> lock(datastoreMutex);
        saveFile->writeData(timestamps, rawValues, clampValues);
    }
	if (saveFileAux) {
		saveFileAux->writeDataAux(timestamps, adcs, numAdcs, digIns, digOuts);
	}
}

void DataStore::setOverlay(bool value) {
    overlay = value;
    emit timescaleChanged();

    reinitAll();
    handleChange(true, true); // Hack: dataChanged should really be false, but reinitAll clears all the data.
}

void DataStore::init(const SimplifiedWaveform& simplifiedWaveform_, bool applyVoltages_) {
    lock_guard<recursive_mutex> lock(datastoreMutex);

    applyVoltages = applyVoltages_;
    simplifiedWaveform = simplifiedWaveform_;
    emit timescaleChanged();

    reinitAll();
}

void DataStore::startCycle() {
    lock_guard<recursive_mutex> lock(datastoreMutex);

    clear();
    resetAll();
}

void DataStore::clear() {
    lock_guard<recursive_mutex> lock(datastoreMutex);

    rawValues.erase(rawValues.begin(), rawValues.end());
	clampValues.erase(clampValues.begin(), clampValues.end());
    timestamps.erase(timestamps.begin(), timestamps.end());
	digIns.erase(digIns.begin(), digIns.end());
	digOuts.erase(digOuts.begin(), digOuts.end());
    adcs.erase(adcs.begin(), adcs.end());
    adcs.resize(8);
    adcsDouble.erase(adcsDouble.begin(), adcsDouble.end());
    adcsDouble.resize(8);
}

void DataStore::reinitAll() {
    lock_guard<recursive_mutex> lock(datastoreMutex);
    for (auto& processor : waveformProcessors) {
        processor->init();
    }
    startAt = 0;
}

void DataStore::resetAll() {
    lock_guard<recursive_mutex> lock(datastoreMutex);
    for (auto& processor : waveformProcessors) {
        processor->reset();
    }
    startAt = 0;
}

void DataStore::storeData(const vector<double>& values_, const vector<double>& clampValues_, double absoluteTime_) {
    lock_guard<recursive_mutex> lock(datastoreMutex);

    rawValues.insert(rawValues.end(), values_.begin(), values_.end());
	clampValues.insert(clampValues.end(), clampValues_.begin(), clampValues_.end());

    const vector<uint32_t>& timestamps_ = state->board->readQueue.getTimeStamps();
    if (timestamps.empty() && !timestamps_.empty()) {
        cycleStartTime = timestamps_.front() / state->board->getSamplingRateHz();
    }
    timestamps.insert(timestamps.end(), timestamps_.begin(), timestamps_.end());

	const vector<uint16_t>& digIns_ = state->board->readQueue.getDigIns();
	digIns.insert(digIns.end(), digIns_.begin(), digIns_.end());

	const vector<uint16_t>& digOuts_ = state->board->readQueue.getDigOuts();
	digOuts.insert(digOuts.end(), digOuts_.begin(), digOuts_.end());

    const vector<vector<uint16_t>>& adcs_ = state->board->readQueue.getADCs();
    for (unsigned int i = 0; i < adcs_.size(); i++) {
        adcs[i].insert(adcs[i].end(), adcs_[i].begin(), adcs_[i].end());
        for (uint16_t value : adcs_[i]) {
            adcsDouble[i].push_back(value * STEPADC);
        }
    }

    absoluteTime = absoluteTime_;
    handleChange(false, true);
}

// Enable or disable low-pass filter.
void DataStore::enableLowPassFilter(bool enable)
{
    emit lowPassFilterEnabledChanged(enable);
    resetAll();
    handleChange(false, true);
}

// Set low-pass filter cutoff frequency (in Hz).
void DataStore::setLowPassFilterCutoff(double fc)
{
    emit lowPassFilterCutoffChanged(fc);
    resetAll();
    handleChange(false, true);
}

void DataStore::handleChange(bool overlayChanged, bool dataChanged) {
    lock_guard<recursive_mutex> lock(datastoreMutex);
    for (auto& processor : waveformProcessors) {
        processor->process(overlayChanged, dataChanged);
    }

    // This is true at the end of a cycle
    if (dataChanged && !simplifiedWaveform.waveform.empty() && (rawValues.size() > simplifiedWaveform.waveform.back().endIndex)) {
        emit waveformDone();
        writeToFile();
    }

    startAt = rawValues.size();
}

bool DataStore::dataAvailable(unsigned int segmentNumber) {
    lock_guard<recursive_mutex> lock(datastoreMutex);
    return rawValues.size() > simplifiedWaveform.waveform[segmentNumber].endIndex;
}

void DataStore::fillSaveHeader(HeaderData& header, int unit, bool holdingOnly, unsigned int lastIndex) {
    controlWindow->fillSettings(header.settings, unit, holdingOnly, lastIndex);
    displayWindow->fillSettings(header.settings);
}

void DataStore::setProcessors(std::vector<std::unique_ptr<DataProcessor>>& waveformProcessors_) {
    lock_guard<recursive_mutex> lock(datastoreMutex);

    waveformProcessors = std::move(waveformProcessors_);
    reinitAll();
    handleChange(false, true);
}

void DataStore::setWholeCell(double Ra, double Rm, double Cm) {
    if (cellParametersValue.value()) {
        this->Ra = Ra;
		this->Rm = Rm;
		this->Cm = Cm;
        emit wholeCellParametersChanged(Ra, Rm, Cm);
    }
}

void DataStore::adjustRa(double RaMOhms) {
	// if (cellParametersValue.value()) {
	this->Ra = RaMOhms * 1.0e6;
	// }
}

