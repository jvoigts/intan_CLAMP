#include "DisplayWindow.h"
#include "SaveFile.h"
#include <QtGui>
#include "common.h"
#include "Thread.h"
#include "ClampThread.h"
#include "ResistanceWidget.h"
#include "globalconstants.h"
#include "Board.h"
#include "ControlWindow.h"
#include "Line.h" // TEMP

using namespace CLAMP;
using namespace CLAMP::IO;
using std::exception;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::deque;

//--------------------------------------------------------------------------
// Constructor.
DisplayWindow::DisplayWindow(GlobalState& state_, const std::string& calibrationReport_, int unit_) :
    state(state_),
    calibrationReport(std::move(calibrationReport_)),
	unit(unit_)
{
    LOG(true) << calibrationReport.c_str();

	lastUnitRun = unit;
	validFilename = false;

    createActions();
    // createMenus();
    createStatusBar();
    createLayout();

    autoScaleCheckBox->setChecked(true);

    connect(&state, SIGNAL(error(const char*, const char*)), this, SLOT(errorMessage(const char*, const char*)));
    connect(&state.datastore[unit_], SIGNAL(timescaleChanged()), this, SLOT(adjustTAxis()));
    connect(autoScaleCheckBox, SIGNAL(toggled(bool)), this, SLOT(adjustTAxis()));
}

DisplayWindow::~DisplayWindow() {
}

// Create GUI layout.  We create the user interface (UI) directly with code, so it
// is not necessary to use Qt Designer or any other special software to modify the
// UI.
void DisplayWindow::createLayout()
{
    setWindowIcon(QIcon(":/images/Intan_Icon_32p_trans24.png"));
    setWindowTitle(tr("Intan Technologies CLAMP Controller - Data Display"));

    createControls();
    layout = new QVBoxLayout;
    layout->addItem(controls);

    const int MAX_NUM_X_STEPS = 10;
    const int MAX_NUM_Y_STEPS = 10;
    tAxis.reset(new Axis(MAX_NUM_X_STEPS, 4, "s", "time", 2, -5, 1, 3, true));  // 20e-6 to 1e3 - minimum corresponds to 1/50 kHz
    tAxis->autoscalable = false;
    tAxis2.reset(new Axis(MAX_NUM_X_STEPS, 4, "s", "time", 1, -3, 1, 3, true));  // 1e-3 to 1e3
    rAxis.reset(new Axis(MAX_NUM_Y_STEPS, 9, QSTRING_OMEGA_SYMBOL, "measured resistance", 1, 3, 1, 10, true)); // 1e3 .. 10e9
    appliedVAxis.reset(new Axis(MAX_NUM_Y_STEPS, 11, "V", "clamp voltage", 1, -5, 5, -1, false)); // 10e-6 .. 0.5
    appliedIAxis.reset(new Axis(MAX_NUM_Y_STEPS, 6, "A", "clamp current", 2, -12, 2, -4, false)); // 2e-12 .. 200e-6
    measuredIAxis.reset(new Axis(MAX_NUM_Y_STEPS, 6, "A", "measured current", 2, -12, 2, -4, false)); // 2e-12 .. 200e-6
    measuredVAxis.reset(new Axis(MAX_NUM_Y_STEPS, 11, "V", "measured voltage", 1, -5, 5, -1, false)); // 10e-6 .. 0.5

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(layout);

    setCentralWidget(mainWidget);

    connect(&state, SIGNAL(threadStatusChanged(bool)), this, SLOT(setThreadStatus(bool)));
}

void DisplayWindow::setUnit(int unit_)
{
	disconnect(&state.datastore[unit], SIGNAL(timescaleChanged()), this, SLOT(adjustTAxis()));
	disconnect(overlayCheckBox, SIGNAL(toggled(bool)), &state.datastore[unit], SLOT(setOverlay(bool)));
	unit = unit_;
	connect(&state.datastore[unit], SIGNAL(timescaleChanged()), this, SLOT(adjustTAxis()));
	connect(overlayCheckBox, SIGNAL(toggled(bool)), &state.datastore[unit], SLOT(setOverlay(bool)));
	state.datastore[unit].setOverlay(overlayCheckBox->isChecked());
}

// Create QActions linking menu items to functions.
void DisplayWindow::createActions()
{
    viewCalibrationReportAction = new QAction(tr("Calibration Report"), this);
    connect(viewCalibrationReportAction, SIGNAL(triggered()), this, SLOT(viewCalibrationReport()));
}

// Create pull-down menus.
void DisplayWindow::createMenus()
{
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(viewCalibrationReportAction);
}

// Create status bar at bottom of the main window.
void DisplayWindow::createStatusBar()
{
}

void DisplayWindow::closeEvent(QCloseEvent *event)
{
    // Perform any clean-up here before application closes.
    event->accept();
}

void DisplayWindow::viewCalibrationReport() {
    QMessageBox::information(this, "Calibration Report", calibrationReport.c_str());
}


void DisplayWindow::fillSettings(Settings& settings) {
    const double freqs[] = { 0, 1000.0, 2000.0, 5000.0, 10000.0 };
    settings.filterCutoff = freqs[lowPassFilterComboBox->currentIndex()];
}

void DisplayWindow::createControls() {
    autoScaleCheckBox = new QCheckBox(tr("Autoscale"));

    overlayCheckBox = new QCheckBox(tr("Overlay multi-steps"));
    connect(overlayCheckBox, SIGNAL(toggled(bool)), &state.datastore[unit], SLOT(setOverlay(bool)));
    overlayCheckBox->setChecked(true);

    saveButton = new QPushButton(tr("Save Displayed"), this);
    saveButton->setToolTip(tr("Save the waveforms and settings currently displayed"));
	saveButton->setEnabled(false);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));

    clearButton = new QPushButton("Clear");
	clearButton->setToolTip(tr("Clear display"));

    controls = new QHBoxLayout;
    controls->addItem(createFilteringLayout());
    controls->addStretch(1);
    controls->addWidget(autoScaleCheckBox);
    controls->addStretch(1);
    controls->addWidget(overlayCheckBox);
    controls->addStretch(1);
    controls->addWidget(saveButton);
    controls->addStretch(1);
    controls->addWidget(clearButton);
    controls->addStretch(1);
}

void DisplayWindow::connectPlot(Plot* plot, int oldUnit) {
    connect(clearButton, SIGNAL(clicked()), &plot->data, SLOT(clearLines()));
    connect(autoScaleCheckBox, SIGNAL(toggled(bool)), plot, SLOT(setAutoScaling(bool)));
	disconnect(&state.datastore[oldUnit], SIGNAL(waveformDone()), plot, SLOT(autoScaleForce()));  
    connect(&state.datastore[unit], SIGNAL(waveformDone()), plot, SLOT(autoScaleForce()));  
}

void DisplayWindow::recreateDisplayLayout(int oldUnit) {
    layout->removeItem(controls);

    for (auto& plot : plots)
    {
        connectPlot(plot.get(), oldUnit);
        plot->setAutoScaling(autoScaleCheckBox->isChecked());
        layout->addWidget(plot.get());
    }
    layout->addItem(controls);
}

void DisplayWindow::save()
{
    try {
		// Create subdirectory for individual headstage data files
		QFileInfo fileInfo(saveBaseFileName);
		QDateTime dateTime = QDateTime::currentDateTime();

		QString subdirName;
		subdirName = fileInfo.baseName();
		subdirName += "_";
		subdirName += dateTime.toString("yyMMdd");    // date stamp
		subdirName += "_";
		subdirName += dateTime.toString("HHmmss");    // time stamp

		QDir dir(fileInfo.path());
		dir.mkdir(subdirName);

		QDir subdir(fileInfo.path() + "/" + subdirName);

		unsigned int lastIndex = state.datastore[lastUnitRun].simplifiedWaveform.lastIndex(state.datastore[lastUnitRun].overlay);
		bool saveAux = true;
		for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
			if (state.board->chip[i]->present) {
				state.datastore[i].openFile(subdirName, saveBaseFileName, dateTime, i, saveAux & state.saveAuxMode);
				saveAux = false;
				if (i == lastUnitRun) {
					state.datastore[i].writeHeader(i);
				}
				else {
					state.datastore[i].writeHeader(i, true, lastIndex);
				}
				state.datastore[i].writeToFile();
				state.datastore[i].closeFile();
			}
		}
		QMessageBox::information(this, "Data Saved", "Data successfully saved");
    }
    catch (exception& e) {
        QMessageBox::critical(this, "Error opening save file", e.what());
    }
}

void DisplayWindow::setThreadStatus(bool running) {
    saveButton->setEnabled(!running && validFilename);
}

template<class T>
void DisplayWindow::add(vector<unique_ptr<DataProcessor>>& waveformProcessors, std::unique_ptr<T>& processor) {
    if (processor.get() != nullptr) {
        waveformProcessors.push_back(std::move(processor));
    }
}

void DisplayWindow::adjustTAxis() {
    if (autoScaleCheckBox->isChecked()) {
        tAxis->scale(0, state.datastore[unit].simplifiedWaveform.lastIndex(state.datastore[unit].overlay) / state.board->getSamplingRateHz());
    }
}

void DisplayWindow::setupPlotsAndCalculations(int oldUnit) {
    vector<unique_ptr<DataProcessor>> waveformProcessorsTmp;
    deque<unique_ptr<Plot>> plotsTmp;

    plotsTmp.clear();
	unique_ptr<FilterProcessor> filter(new FilterProcessor(state.datastore[unit], state.datastore[unit].rawValues));
    changeLowPassFilter(lowPassFilterComboBox->currentIndex());

    unique_ptr<AppliedWaveformProcessor> applied;
	unique_ptr<AppliedPlusAdcProcessor> appliedPlusAdc;
    unique_ptr<VCellProcessor> vcell;

    if (config.appliedPlot | config.showAppliedPlusAdc) {
        applied.reset(new AppliedWaveformProcessor(state.datastore[unit]));
		
		if (config.showAppliedPlusAdc) {
			appliedPlusAdc.reset(new AppliedPlusAdcProcessor(state.datastore[unit], *applied, state.datastore[unit].clampValues));
		}

        shared_ptr<Axis> yAxis;
        if (config.appliedVoltage) {
            yAxis = appliedVAxis;
            if (config.showVcell) {
                vcell.reset(new VCellProcessor(state.datastore[unit], *applied, *filter));
            }
        }
        else {
            yAxis = appliedIAxis;
        }

		plotsTmp.push_front(std::move(unique_ptr<Plot>(new Plot(this, applied->waveforms, tAxis, yAxis))));
    }

    unique_ptr<MeasuredWaveformProcessor> measured;
    if (config.measuredPlot()) {
        shared_ptr<Axis> yAxis;

        if (config.measuredCurrent) {
            measured.reset(new MeasuredWaveformProcessor(state.datastore[unit], *filter, false));
            yAxis = measuredIAxis;
        }
        else {
            measured.reset(new MeasuredWaveformProcessor(state.datastore[unit], *filter, config.showBridgeBalance));
            yAxis = measuredVAxis;
        }
        plotsTmp.push_front(std::move(unique_ptr<Plot>(new Plot(this, measured->waveforms, tAxis, yAxis))));
    }

    unique_ptr<DCCalculationProcessor> dcCalculation;
    unique_ptr<DCPlotProcessor> dcPlotProcessor;
    unique_ptr<ExponentialCalculationWaveformProcessor> exp;
    unique_ptr<ExponentialPlotProcessor> expPlot;
    unique_ptr<ResistanceCalculationWaveformProcessor> resistanceCalculations;
    unique_ptr<CellParameterProcessor> cellParameters;

    if (config.dcCalculation()) {
        dcCalculation.reset(new DCCalculationProcessor(state.datastore[unit], *filter));
    }
    if (config.showDCCalculations) {
        dcPlotProcessor.reset(new DCPlotProcessor(state.datastore[unit], measured->waveforms, *dcCalculation));
    }
    if (config.fitExponentials()) {
        exp.reset(new ExponentialCalculationWaveformProcessor(state.datastore[unit], *filter));
    }
    if (config.showFittedExponentials) {
        expPlot.reset(new ExponentialPlotProcessor(state.datastore[unit], measured->waveforms, *exp));
    }
    if (config.resistanceCalculation()) {
        resistanceCalculations.reset(new ResistanceCalculationWaveformProcessor(state.datastore[unit], dcCalculation.get(), exp.get()));
    }
    if (config.cellParametersCalculation) {
        cellParameters.reset(new CellParameterProcessor(state.datastore[unit], *exp));
    }

    add(waveformProcessorsTmp, filter);
    add(waveformProcessorsTmp, applied);
	add(waveformProcessorsTmp, appliedPlusAdc);
    add(waveformProcessorsTmp, vcell);
    add(waveformProcessorsTmp, measured);
    add(waveformProcessorsTmp, dcCalculation);
    add(waveformProcessorsTmp, dcPlotProcessor);
    add(waveformProcessorsTmp, exp);
    add(waveformProcessorsTmp, expPlot);
    add(waveformProcessorsTmp, resistanceCalculations);
    add(waveformProcessorsTmp, cellParameters);

    if (config.resistancePlot) {
        unique_ptr<ResistanceProcessor> resistance(new ResistanceProcessor(state.datastore[unit]));
        plotsTmp.push_front(std::move(unique_ptr<Plot>(new Plot(this, resistance->waveform, tAxis2, rAxis))));
        add(waveformProcessorsTmp, resistance);
    }

    plots.swap(plotsTmp);
    recreateDisplayLayout(oldUnit);
	
    plotsTmp.clear();
    state.datastore[unit].setProcessors(waveformProcessorsTmp);
}


void DisplayWindow::setPlotOptions(const PlotConfiguration& config_, int unit_) {
	int oldUnit = unit;
    config = config_;
	if (unit != unit_) {
		setUnit(unit_);
	}
	// unit = unit_;  // redundant with above setUnit call
    setupPlotsAndCalculations(oldUnit);
}

void DisplayWindow::changeLowPassFilter(int index)
{
    state.datastore[unit].enableLowPassFilter(index != 0);
    if (index != 0) {
        const double freqs[] = { 0, 1000.0, 2000.0, 5100.0, 10800.0 };
        state.datastore[unit].setLowPassFilterCutoff(freqs[index]);
    }
}

QLayoutItem* DisplayWindow::createFilteringLayout() {
    lowPassFilterComboBox = new QComboBox();
    lowPassFilterComboBox->addItem(tr("Disabled"));
    lowPassFilterComboBox->addItem(tr("1 kHz"));
    lowPassFilterComboBox->addItem(tr("2 kHz"));
    lowPassFilterComboBox->addItem(tr("5 kHz"));
    lowPassFilterComboBox->addItem(tr("10 kHz"));
    lowPassFilterComboBox->setCurrentIndex(4);

    QHBoxLayout *lowPassFilterLayout = new QHBoxLayout;
    lowPassFilterLayout->addWidget(new QLabel("Low-pass filter:"));
    lowPassFilterLayout->addWidget(lowPassFilterComboBox);
    lowPassFilterLayout->addWidget(new QLabel(" (Bessel, 80 dB / decade)"));
    lowPassFilterLayout->addStretch(1);

    connect(lowPassFilterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLowPassFilter(int)));

    return lowPassFilterLayout;
}

void DisplayWindow::errorMessage(const char* title, const char* message) {
    QMessageBox::information(this, title, message);
}

void DisplayWindow::setLastUnitRun(int lastUnitRun_) {
	lastUnitRun = lastUnitRun_;
}

void DisplayWindow::setFilename(QString filename) {
	saveBaseFileName = filename;
	validFilename = true;
	saveButton->setEnabled(true);
}
