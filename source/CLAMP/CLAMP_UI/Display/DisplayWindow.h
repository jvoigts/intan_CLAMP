#pragma once

#include <QMainWindow>
#include <memory>
#include "GlobalState.h"
#include <deque>

class ControlWindow;
class DisplayWidget;
class QLayoutItem;
class QPushButton;
class QCheckBox;
class QComboBox;
class QHBoxLayout;
class QVBoxLayout;

namespace CLAMP {
    class Board;
    namespace IO {
        struct Settings;
    }
}

struct PlotConfiguration {
    // Plot of applied voltage or current
    bool appliedPlot;
    bool appliedVoltage;
    bool showVcell;  // In voltage clamp mode, show the V-cell plot
	bool showAppliedPlusAdc;

    // Plot of measured voltage or current
    bool measuredCurrent;
    bool showBridgeBalance;  // In current clamp mode, adjust the measured voltage to compensate for series resistance
    bool showMeasuredData;
    bool showDCCalculations;
    bool showFittedExponentials;
    bool measuredPlot() const { return showMeasuredData || showDCCalculations || showFittedExponentials; }

    // Ra, Rm, Cm
    bool cellParametersCalculation;

    // Current Resistance Value
    bool displayResistance;

    // Resistance vs. time
    bool resistancePlot;

    // Calculations
    bool fitExponentials() const { return showFittedExponentials || cellParametersCalculation; }
    bool resistanceCalculation() const { return displayResistance || cellParametersCalculation || resistancePlot; }
    bool dcCalculation() const { return resistanceCalculation() || showDCCalculations; }
};

class DisplayWindow : public QMainWindow
{
    Q_OBJECT

    GlobalState& state;
public:
    DisplayWindow(GlobalState& state_, const std::string& calibrationReport_, int unit_);
    ~DisplayWindow();

    void fillSettings(CLAMP::IO::Settings& settings);
    void setPlotOptions(const PlotConfiguration& config_, int unit_);
	void setUnit(int unit_);
	void setLastUnitRun(int lastUnitRun_);
	void setFilename(QString filename);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void viewCalibrationReport();
    void changeLowPassFilter(int index);
    void save();
    void setThreadStatus(bool running);
    void errorMessage(const char* title, const char* message);
    void adjustTAxis();

private:
    DisplayWindow(const DisplayWindow&); // Don't do it

	int unit;
	int lastUnitRun;
	QString saveBaseFileName;
	bool validFilename;

    void createActions();
    void createMenus();
    void createStatusBar();
    void createLayout();

    std::string calibrationReport;
    QAction *viewCalibrationReportAction;

    QVBoxLayout* layout;
    void recreateDisplayLayout(int oldUnit);

    void createControls();
    QHBoxLayout* controls;

    // Save
    QPushButton* saveButton;

    // Filtering
    QLayoutItem* createFilteringLayout();
    QComboBox *lowPassFilterComboBox;

    // Other
    QCheckBox* autoScaleCheckBox;
    QCheckBox* overlayCheckBox;
    QPushButton* clearButton;

    std::deque<std::unique_ptr<Plot>> plots;
    // Various axes, so that we can keep the zoom level(s)
    std::shared_ptr<Axis> tAxis;
    std::shared_ptr<Axis> tAxis2;
    std::shared_ptr<Axis> rAxis;
    std::shared_ptr<Axis> appliedVAxis;
    std::shared_ptr<Axis> measuredVAxis;
    std::shared_ptr<Axis> appliedIAxis;
    std::shared_ptr<Axis> measuredIAxis;

    void connectPlot(Plot* plot, int oldUnit);

    PlotConfiguration config;
    void setupPlotsAndCalculations(int oldUnit);

    template<class T>
    void add(std::vector<std::unique_ptr<DataProcessor>>& waveformProcessors, std::unique_ptr<T>& processor);
};
