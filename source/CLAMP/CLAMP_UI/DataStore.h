#pragma once

#include "Plot.h"
#include <memory>
#include "ClampThread.h"
#include "MVC.h"
#include "streams.h"

class QDateTime;

namespace CLAMP {
    namespace SignalProcessing {
        class Filter;
    }
    namespace IO {
        class SaveFile;
        struct HeaderData;
    }
}

struct DCParameters {
    double steadyStateValue;
    bool valid;

    DCParameters() : valid(false) {}
};

struct ExponentialParameters {
    double beta[3];
    bool valid;

    ExponentialParameters() : valid(false) {}
};

class DataStore;

class DataProcessor : public QObject {
    Q_OBJECT

public:
    DataProcessor(DataStore& datastore_);
    virtual ~DataProcessor() {}

    virtual void init() = 0;
    virtual void reset() = 0;
    virtual void process(bool overlayChanged, bool dataChanged) = 0;

protected:
    DataStore& datastore;
};

class AppliedWaveformProcessor : public DataProcessor {
public:
    AppliedWaveformProcessor(DataStore& datastore_);
    ~AppliedWaveformProcessor();

    Lines waveforms;

    void init() override;
    void reset() override;
    void process(bool overlayChanged, bool dataChanged) override;

private:
    std::vector<Line> appliedWaveforms;

    void getAppliedWaveforms(double samplingRate);
    void createAppliedWaveformPlot();
};

typedef double(*correctFunction)(bool correct, double applied, double value, double r);
LineIncrements seriesResistanceCorrect(DataStore& datastore, const std::vector<double>& values, double samplingRate, bool doCorrection, correctFunction f);

class FilterProcessor;
class AppliedPlusAdcProcessor : public DataProcessor {
public:
	AppliedPlusAdcProcessor(DataStore& datastore_, AppliedWaveformProcessor& applied_, std::vector<double>& clampValues_);
	~AppliedPlusAdcProcessor();

	void init() override {}
	void reset() override {}
	void process(bool overlayChanged, bool dataChanged) override;

private:
	AppliedWaveformProcessor& applied;
	std::vector<double>& clampValues;

	static double dummyFunction(bool correct, double applied, double value, double r);
};

class VCellProcessor : public DataProcessor {
public:
    VCellProcessor(DataStore& datastore_, AppliedWaveformProcessor& applied_, FilterProcessor& source_);
    ~VCellProcessor();

    void init() override {}
    void reset() override {}
    void process(bool overlayChanged, bool dataChanged) override;

private:
    AppliedWaveformProcessor& applied;
    FilterProcessor& source;

    static double vCellCorrect(bool correct, double applied, double value, double r);
};

class MeasuredWaveformProcessor : public DataProcessor {
public:
    MeasuredWaveformProcessor(DataStore& datastore_, FilterProcessor& filter, bool bridgeBalance_);
    ~MeasuredWaveformProcessor();

    Lines waveforms;

    void init() override;
    void reset() override;
    void process(bool overlayChanged, bool dataChanged) override;

private:
    static double bridgeBalanceCorrect(bool correct, double applied, double value, double r);
    LineIncrements getMeasuredWaveforms(const std::vector<double>& values, double samplingRate);

    FilterProcessor& source;
    bool bridgeBalance;
};

class DCCalculationProcessor : public DataProcessor {
public:
    DCCalculationProcessor(DataStore& datastore_, FilterProcessor& filter);
    ~DCCalculationProcessor();

    void init() override;
    void reset() override;
    void process(bool overlayChanged, bool dataChanged) override;

    std::vector<DCParameters> waveformCalculations;

private:
    FilterProcessor& source;

    void getSteadyStateValues(const std::vector<double>& values);
};

class DCPlotProcessor : public DataProcessor {
public:
    DCPlotProcessor(DataStore& datastore_, Lines& waveforms_, DCCalculationProcessor& calc);
    ~DCPlotProcessor();

    void init() override;
    void reset() override;
    void process(bool overlayChanged, bool dataChanged) override;

private:
    Lines& waveforms;
    DCCalculationProcessor& dcCalculation;

    LineIncrements getResidualWaveforms(double samplingRate);
};

class ExponentialCalculationWaveformProcessor : public DataProcessor {
public:
    ExponentialCalculationWaveformProcessor(DataStore& datastore_, FilterProcessor& filter);
    ~ExponentialCalculationWaveformProcessor();

    void init() override;
    void reset() override;
    void process(bool overlayChanged, bool dataChanged) override;

    std::vector<ExponentialParameters> exponentialParameters;

private:
    FilterProcessor& source;

    void fitExponentials(const std::vector<double>& values, double samplingRate);
};

class ExponentialPlotProcessor : public DataProcessor {
public:
    ExponentialPlotProcessor(DataStore& datastore_, Lines& waveforms_, ExponentialCalculationWaveformProcessor& calc);
    ~ExponentialPlotProcessor();

    void init() override;
    void reset() override;
    void process(bool overlayChanged, bool dataChanged) override;

private:
    Lines& waveforms;
    ExponentialCalculationWaveformProcessor& calculator;

    LineIncrements getExponentialWaveforms(double samplingRate);
};

class ResistanceCalculationWaveformProcessor : public DataProcessor {
public:
    ResistanceCalculationWaveformProcessor(DataStore& datastore_, DCCalculationProcessor* dc_, ExponentialCalculationWaveformProcessor* exp_);
    ~ResistanceCalculationWaveformProcessor();

    void init() override {}
    void reset() override {}
    void process(bool overlayChanged, bool dataChanged) override;

private:
    DCCalculationProcessor* dc;
    ExponentialCalculationWaveformProcessor* exp;

    static double calculateR(const std::vector<double>& currents, const std::vector<double>& voltages);
};

class CellParameterProcessor : public DataProcessor {
public:
    CellParameterProcessor(DataStore& datastore_, ExponentialCalculationWaveformProcessor& exp_);
    ~CellParameterProcessor();

    void init() override {}
    void reset() override {}
    void process(bool overlayChanged, bool dataChanged) override;

private:
    ExponentialCalculationWaveformProcessor& exp;

    void calculateCellParameters(double& Ra, double& Rm, double& Cm);
    static void getRsAndCs(double beta[3], double dV, double resistance, double& Ra, double& Rm, double& Cm);
};

class ResistanceProcessor : public DataProcessor {
public:
    ResistanceProcessor(DataStore& datastore_);
    ~ResistanceProcessor();

    Lines waveform;

    void init() override {}
    void reset() override {}
    void process(bool overlayChanged, bool dataChanged) override;
};

class FilterProcessor : public DataProcessor {
    Q_OBJECT

public:
    FilterProcessor(DataStore& datastore_, std::vector<double>& rawValues_);
    ~FilterProcessor();

    void init() override;
    void reset() override;
    void process(bool overlayChanged, bool dataChanged) override;

    const std::vector<double>& getValues();

public slots:
    void enableLowPassFilter(bool enable);
    void setLowPassFilterCutoff(double fc);

private:
    std::unique_ptr<CLAMP::SignalProcessing::Filter> filter;
    bool lowPassFilterEnabled;
    std::vector<double> filteredValues;
    std::vector<double>& rawValues;
};

class DisplayWindow;
class ControlWindow;
class GlobalState;
class DataStore : public QObject {
    Q_OBJECT

public:
    //DataStore(GlobalState& state_);
	DataStore();
    ~DataStore();

    void openFile(const QString& subdirName, const QString& baseFilename, const QDateTime& dateTime, int unit, bool auxDataToo);
    void writeHeader(int unit, bool holdingOnly = false, unsigned int lastIndex = 0);
    void writeToFile();
    void closeFile();
    void init(const CLAMP::SimplifiedWaveform& simplifiedWaveform, bool applyVoltages);
    void startCycle();
    void clear();
    void storeData(const std::vector<double>& values, const std::vector<double>& clampValues, double absoluteTime);

    void enableLowPassFilter(bool enable);
    void setLowPassFilterCutoff(double fc);
    void setProcessors(std::vector<std::unique_ptr<DataProcessor>>& waveformProcessors_);

    double resistance;

public slots:
    void setOverlay(bool value);
    void setWholeCell(double Ra, double Rm, double Cm);
	void adjustRa(double Ra);

signals:
    void lowPassFilterEnabledChanged(bool enable);
    void lowPassFilterCutoffChanged(double fc);
    void wholeCellParametersChanged(double Ra, double Rm, double Cm);
    void timescaleChanged();
    void waveformDone();

public:
    DisplayWindow* displayWindow;
    ControlWindow* controlWindow;
    GlobalState* state;

    std::vector<double> rawValues;
	std::vector<double> clampValues;
    std::vector<std::vector<double>> adcsDouble;
    double cycleStartTime;
    std::vector<uint32_t> timestamps;
	std::vector<uint16_t> digIns;
	std::vector<uint16_t> digOuts;
    CLAMP::SimplifiedWaveform simplifiedWaveform;
    double absoluteTime;

    bool dataAvailable(unsigned int segmentNumber);

    bool applyVoltages;
    bool overlay;
    unsigned int startAt;

    BoolHolder cellParametersValue;
    double Ra;
	double Rm;
	double Cm;

private:
    CLAMP::IO::SaveFile* saveFile;
	CLAMP::IO::SaveFile* saveFileAux;
    std::vector<Line> waveforms;
    std::vector<std::vector<uint16_t>> adcs;
	int numAdcs;

    std::recursive_mutex datastoreMutex; // Acquire this when you need to be threadsafe
    std::vector<std::unique_ptr<DataProcessor>> waveformProcessors;

    void handleChange(bool overlayChanged, bool dataChanged);
    void resetAll();
    void reinitAll();
    void fillSaveHeader(CLAMP::IO::HeaderData& header, int unit, bool holdingOnly = false, unsigned int lastIndex = 0);
};
