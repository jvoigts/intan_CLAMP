#include "ControlWindow.h"
#include <QtGui>
#include "common.h"
#include "GUIUtil.h"
#include "globalconstants.h"
#include "Board.h"
#include "Constants.h"
#include "GlobalState.h"
#include "WaveformAmplitudeWidget.h"
#include "ClampThread.h"
#include "ExternalCommandWidget.h"
#include "SignalOutputWidget.h"
#include "MarkerOutputWidget.h"
#include "ResistanceWidget.h"
#include "HoldingVoltageWidget.h"
#include "SaveFile.h"
#include "DisplayWindow.h"
#include "VoltageClampWidget.h"
#include "CurrentClampWidget.h"
#include "PipetteOffset.h"

using std::exception;
using std::unique_ptr;
using namespace CLAMP::IO;

//--------------------------------------------------------------------------
ControlWindow::ControlWindow(QWidget* parent, GlobalState& state_) :
    QMainWindow(parent),
    state(state_),
    enableTabbing(true)
{
    createActions();
    createMenus();
    createStatusBar();
    createLayout();

    connect(this, SIGNAL(clampModeChanged(int)), this, SLOT(setClampMode(int)));
    connect(&state, SIGNAL(threadStatusChanged(bool)), this, SLOT(setThreadStatus(bool)));
    connect(&state, SIGNAL(statusMessage(int, QString)), this, SLOT(setStatusMessage(int, QString)));

	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		connect(appliedVoltageWaveform[i], SIGNAL(changeDisplay()), this, SLOT(setDisplayOptions()));
		connect(appliedCurrentWaveform[i], SIGNAL(changeDisplay()), this, SLOT(setDisplayOptions()));
	}

	runningHeadstage = 0;
	validFilename = false;
}

ControlWindow::~ControlWindow() {
    // Be sure this happens before we get rid of objects
    state.closeThreads();
}

void ControlWindow::createActions() {
    // measureTemperatureAction = new QAction(tr("Measure Temperature"), this);
    // connect(measureTemperatureAction, SIGNAL(triggered()), this, SLOT(measureTemperature()));
	keyboardHelpAction = new QAction(tr("&Keyboard Shortcuts..."), this);
	keyboardHelpAction->setShortcut(tr("F1"));
	connect(keyboardHelpAction, SIGNAL(triggered()), this, SLOT(keyboardShortcutsHelp()));
	intanWebsiteAction = new QAction(tr("Visit Intan Website..."), this);
	connect(intanWebsiteAction, SIGNAL(triggered()), this, SLOT(openIntanWebsite()));
	aboutAction = new QAction(tr("&About Intan GUI..."), this);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
	saveAuxAction = new QAction("Save Auxiliary I/O", this);
	saveAuxAction->setCheckable(true);
	saveAuxAction->setChecked(true);
	connect(saveAuxAction, SIGNAL(toggled(bool)), this, SLOT(setSaveAux(bool)));
	vClampX2Action = new QAction(tr("2x Voltage Clamp Mode"), this);
	vClampX2Action->setCheckable(true);
	vClampX2Action->setChecked(false);
	connect(vClampX2Action, SIGNAL(toggled(bool)), this, SLOT(setVClampX2(bool)));
}

void ControlWindow::createMenus() {
	QMenu *optionsMenu = menuBar()->addMenu(tr("&Options"));
	optionsMenu->addAction(saveAuxAction);
	optionsMenu->addAction(vClampX2Action);

//    QMenu *actionMenu = menuBar()->addMenu(tr("&Actions"));
//    actionMenu->addAction(measureTemperatureAction);
	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(keyboardHelpAction);
	helpMenu->addSeparator();
	helpMenu->addAction(intanWebsiteAction);
	helpMenu->addAction(aboutAction);
}

void ControlWindow::createStatusBar() {

}

void ControlWindow::createLayout() {
    setWindowIcon(QIcon(":/images/Intan_Icon_32p_trans24.png"));
    setWindowTitle(tr("Intan Technologies CLAMP Controller"));

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(createControlLayout());

    setCentralWidget(mainWidget);
}

void ControlWindow::createCapCompensation() {
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		capCompensationSpinBox[i] = new QDoubleSpinBox();
		capCompensationSpinBox[i]->setRange(0.0, 20.0);
		capCompensationSpinBox[i]->setSingleStep(0.10);
		capCompensationSpinBox[i]->setDecimals(1);
		capCompensationSpinBox[i]->setSuffix(" pF");
		capCompensationSpinBox[i]->setValue(0);
		capCompensationSpinBox[i]->setKeyboardTracking(true);
		connect(capCompensationSpinBox[i], SIGNAL(valueChanged(double)), this, SLOT(setCapacitiveCompensation()));

		QHBoxLayout *layout = new QHBoxLayout;
		layout->addWidget(capCompensationSpinBox[i]);
		layout->addStretch(1);

		capCompensation[i] = new QGroupBox();
		capCompensation[i]->setTitle(tr("Capacitance Compensation"));
		capCompensation[i]->setLayout(layout);

		capCompensation[i]->setCheckable(true);
		capCompensation[i]->setChecked(false);
		connect(capCompensation[i], SIGNAL(toggled(bool)), this, SLOT(enableCapacitiveCompensation(bool)));
	}
}

void ControlWindow::updateStats() {
    fifoLagLabel->setText(QString::number(state.board->latency, 'f', 0) + " ms");
    if (state.board->latency > 50.0) {
        fifoLagLabel->setStyleSheet("color: red");
    }
    else {
        fifoLagLabel->setStyleSheet("color: green");
    }
    fifoLagLabel->update();

    fifoFullLabel->setText("(" + QString::number(state.board->fifoPercentageFull, 'f', 0) + "% full)");
    if (state.board->fifoPercentageFull > 75.0) {
        fifoFullLabel->setStyleSheet("color: red");
    }
    else {
        fifoFullLabel->setStyleSheet("color: black");
    }
    fifoFullLabel->update();
}

double ControlWindow::getCapCompensationValue() const {
    double cp = capCompensationSpinBox[selectedHeadstage()]->value();
    if (tabWidget[selectedHeadstage()]->currentIndex() == 1) { // Current Clamp Mode
		cp -= 0.5; // Optimum value for capacitance compensation is slightly lower in current clamp mode.
    }

    if (cp < 0.0) {
        cp = 0.0;
    }
    return cp * 1e-12;
}

void ControlWindow::setCapacitiveCompensation()
{
    auto& r6 = state.board->chip[selectedHeadstage()]->channel[0]->registers.r6;
	auto& r8 = state.board->chip[selectedHeadstage()]->channel[0]->registers.r8;
    r6.value.setMagnitude(getCapCompensationValue(), r8.value.getStepSize());
}

void ControlWindow::enableCapacitiveCompensation(bool checked)
{
	auto& r6 = state.board->chip[selectedHeadstage()]->channel[0]->registers.r6;
	auto& r7 = state.board->chip[selectedHeadstage()]->channel[0]->registers.r7;
    r7.value.fastTransConnect = checked;
    r6.value.fastTransInSelect = checked && (tabWidget[selectedHeadstage()]->currentIndex() == 1); // Only do this if enabled in current mode
}

void ControlWindow::setResistanceInternal(double value) {
    if (resistanceCheckBox[selectedHeadstage()]->isChecked()) {
        if (value > 1e9) {
            resistanceValue[selectedHeadstage()]->setText(QString::number(value / 1e9, 'f', 1) + " G" + QSTRING_OMEGA_SYMBOL);
        }
        if (value > 1e6) {
            resistanceValue[selectedHeadstage()]->setText(QString::number(value / 1e6, 'f', 1) + " M" + QSTRING_OMEGA_SYMBOL);
        }
        else {
            resistanceValue[selectedHeadstage()]->setText(QString::number(value / 1e3, 'f', 1) + " k" + QSTRING_OMEGA_SYMBOL);
        }
        resistanceValue[selectedHeadstage()]->update();
    }
}

void ControlWindow::setThreadStatus(bool running) {
    stopButton->setEnabled(running);
    runButton->setEnabled(!running);
    runOnceButton->setEnabled(!running);
    recordButton->setEnabled(!running && validFilename);
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		appliedVoltageWaveform[i]->pipetteOffset->autoButton->setEnabled(!running);
		appliedCurrentWaveform[i]->currentStepSizeComboBox->setEnabled(!running);
		appliedCurrentWaveform[i]->zeroCurrentButton->setEnabled(!running);
		if (i != runningHeadstage) {
			capCompensation[i]->setEnabled(false);
			appliedVoltageWaveform[i]->setControlsEnabled(false);
			resistanceCheckBox[i]->setEnabled(false);
			appliedCurrentWaveform[i]->setControlsEnabled(false);
		}
	}
    enableTabbing = !running;

    // These controls may or may not be disabled when we run, but should always be enabled when we don't run
    if (!running) {
		for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
			tabWidget[i]->setEnabled(true);
			capCompensation[i]->setEnabled(true);
			appliedVoltageWaveform[i]->setControlsEnabled(true);
			resistanceCheckBox[i]->setEnabled(true);
			appliedCurrentWaveform[i]->setControlsEnabled(true);
		}
    }
}

DisplayWindow* ControlWindow::getDisplayWindow() const {
    return dynamic_cast<DisplayWindow*>(parent());
}

void ControlWindow::setDisplayOptions() {
    PlotConfiguration config;
	currentTabIndex = tabWidget[selectedHeadstage()]->currentIndex();
    bool isVoltageClamp = (currentTabIndex == 0);
    if (isVoltageClamp) {
        appliedVoltageWaveform[selectedHeadstage()]->getDisplayConfig(config);
    }
    else {
        appliedCurrentWaveform[selectedHeadstage()]->getDisplayConfig(config);
    }
    config.displayResistance = resistanceCheckBox[selectedHeadstage()]->isChecked();
    config.showDCCalculations = config.displayResistance;

    getDisplayWindow()->setPlotOptions(config, selectedHeadstage());
}

void ControlWindow::setClampMode(int index) {
    if (index != currentTabIndex) {
        state.datastore[selectedHeadstage()].clear();
    }

    currentTabIndex = index;

    setDisplayOptions();
}

void ControlWindow::startRecording()
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

		bool saveAux = true;
		for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
			if (state.board->chip[i]->present) {
				state.datastore[i].openFile(subdirName, saveBaseFileName, dateTime, i, saveAux & state.saveAuxMode);
				saveAux = false;
				tabWidget[i]->setEnabled(false);
				capCompensation[i]->setEnabled(false);
			}
		}
        startRunningWithType(ClampThread::CONTINUOUS);
    }
    catch (exception& e) {
        QMessageBox::critical(this, "Error opening save file", e.what());
    }
}

void ControlWindow::startRunning()
{
    startRunningWithType(ClampThread::BATCH);
}

void ControlWindow::startRunningOnce()
{
    startRunningWithType(ClampThread::ONCE);
}

void ControlWindow::startRunningWithType(ClampThread::RunType runtype) {
	runningHeadstage = selectedHeadstage();
	getDisplayWindow()->setLastUnitRun(runningHeadstage);
	unique_ptr<ClampThread> tmp;
	bool voltageClampMode[CLAMP::MAX_NUM_CHIPS];
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		voltageClampMode[i] = (tabWidget[i]->currentIndex() == 0);
	}
	tmp.reset(new ClampThread(state, appliedVoltageWaveform, appliedCurrentWaveform, voltageClampMode, selectedHeadstage()));

    tmp->setRunType(runtype);

    state.runThread(tmp.release());
}

void ControlWindow::stopRunning()
{
    state.stopThreads();
}

QLayoutItem* ControlWindow::createRunStopLayout() {
    // Run/stop/label
    recordButton = new QPushButton(tr("Record"), this);
    connect(recordButton, SIGNAL(clicked()), this, SLOT(startRecording()));
	recordButton->setEnabled(false);

    runButton = new QPushButton(tr("Run"), this);
    connect(runButton, SIGNAL(clicked()), this, SLOT(startRunning()));

    runOnceButton = new QPushButton(tr("Run Once"), this);
    connect(runOnceButton, SIGNAL(clicked()), this, SLOT(startRunningOnce()));

    stopButton = new QPushButton(tr("Stop"), this);
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopRunning()));
    stopButton->setEnabled(false);

	selectFilenameButton = new QPushButton(tr("Select Base Filename"), this);
	connect(selectFilenameButton, SIGNAL(clicked()), this, SLOT(selectFilename()));

	saveFilenameLineEdit = new QLineEdit();
	saveFilenameLineEdit->setEnabled(false);

    QHBoxLayout *hlayout1 = new QHBoxLayout;
	hlayout1->addWidget(recordButton);
	hlayout1->addWidget(runButton);
	hlayout1->addWidget(runOnceButton);
	hlayout1->addWidget(stopButton);
	hlayout1->addStretch(1);

	QHBoxLayout *hlayout2 = new QHBoxLayout;
	hlayout2->addWidget(selectFilenameButton);
	hlayout2->addStretch(1);
	hlayout2->addWidget(new QLabel(tr("Base Filename ")));
	hlayout2->addWidget(saveFilenameLineEdit);


	QVBoxLayout *runStopLayout = new QVBoxLayout;
	runStopLayout->addItem(hlayout1);
	runStopLayout->addStretch(1);
	runStopLayout->addItem(hlayout2);

    return runStopLayout;
}

QLayoutItem* ControlWindow::createFIFOStatsLayout() {
    fifoLagLabel = new QLabel(tr("0 ms"), this);
    fifoLagLabel->setStyleSheet("color: green");

    fifoFullLabel = new QLabel(tr("(0% full)"), this);
    fifoFullLabel->setStyleSheet("color: black");

    connect(this, SIGNAL(updateStatsSignal()), this, SLOT(updateStats()));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch(10);
    layout->addWidget(new QLabel(tr("FIFO lag:")));
    layout->addStretch(1);
    layout->addWidget(fifoLagLabel);
    layout->addWidget(fifoFullLabel);
    layout->addStretch(1);

    return layout;
}

void ControlWindow::setResistance(double value) {
    emit resistanceChanged(value);
}

void ControlWindow::updateStatsExt() {
    emit updateStatsSignal();
}

QWidget* ControlWindow::createStateBox(int unit) {
    statusLabel[unit] = new QLabel("Floating");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(statusLabel[unit]);

    QGroupBox* box = new QGroupBox();
    box->setTitle("Status");
    box->setLayout(layout);
    return box;
}

QLayout* ControlWindow::createControlLayout() {
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		appliedVoltageWaveform[i] = new VoltageClampWidget(state, i);
		appliedCurrentWaveform[i] = new CurrentClampWidget(state, *this, i);
		tabWidget[i] = new QTabWidget();
		tabWidget[i]->addTab(appliedVoltageWaveform[i], tr("Voltage Clamp"));
		tabWidget[i]->addTab(appliedCurrentWaveform[i], tr("Current Clamp"));
		connect(tabWidget[i], SIGNAL(currentChanged(int)), this, SLOT(tryToSetClampMode(int)));
	}

    // Run/stop/label

    createResistanceLayout();
	createExternalCommandLayout();
	createSignalOutputLayout();
    createCapCompensation();

	QHBoxLayout* hLayout1[CLAMP::MAX_NUM_CHIPS];

	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		hLayout1[i] = new QHBoxLayout;
		hLayout1[i]->addWidget(capCompensation[i]);
		hLayout1[i]->addStretch(1);
		hLayout1[i]->addWidget(createStateBox(i));
	}

	headstageTabWidget = new QTabWidget();
	connect(headstageTabWidget, SIGNAL(currentChanged(int)), this, SLOT(setHeadstageFocus(int)));

	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		if (state.board->chip[i]->present) {
			QFrame *frameTab = new QFrame();
			QVBoxLayout *vLayoutTab = new QVBoxLayout;
			vLayoutTab->addItem(hLayout1[i]);
			vLayoutTab->addWidget(tabWidget[i]);
			vLayoutTab->addWidget(resistance[i]);
			QTabWidget* ioWidget = new QTabWidget();
			ioWidget->addTab(externalCommand[i], "External Command");
			ioWidget->addTab(signalOutput[i], "Signal Out");
			connect((QObject*)appliedVoltageWaveform[i]->feedback, SIGNAL(feedbackResistanceChanged(int)), signalOutput[i], SLOT(updateFeedbackResistance(int)));
			ioWidget->addTab(clampOutput[i], "Clamp Out");
			ioWidget->addTab(markerOutput[i], "Marker Out");
			vLayoutTab->addWidget(ioWidget);
			vLayoutTab->addStretch(1);
			frameTab->setLayout(vLayoutTab);
			switch (i) {
			case 0:
				headstageTabWidget->addTab(frameTab, tr("Port A"));
				break;
			case 1:
				headstageTabWidget->addTab(frameTab, tr("Port B"));
				break;
			case 2:
				headstageTabWidget->addTab(frameTab, tr("Port C"));
				break;
			case 3:
				headstageTabWidget->addTab(frameTab, tr("Port D"));
				break;
			case 4:
				headstageTabWidget->addTab(frameTab, tr("Port E"));
				break;
			case 5:
				headstageTabWidget->addTab(frameTab, tr("Port F"));
				break;
			case 6:
				headstageTabWidget->addTab(frameTab, tr("Port G"));
				break;
			case 7:
				headstageTabWidget->addTab(frameTab, tr("Port H"));
				break;
			}
		}
	}

	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addItem(createRunStopLayout());
	// vLayout->addItem(createFIFOStatsLayout());
	vLayout->addWidget(headstageTabWidget);
	vLayout->addStretch(1);

    return vLayout;
}

void ControlWindow::createResistanceLayout() {
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		resistanceValue[i] = new QLabel(tr("-.- M") + QSTRING_OMEGA_SYMBOL, this);
		QFont font = resistanceValue[i]->font();
		font.setPointSize(16);
		font.setBold(true);
		resistanceValue[i]->setFont(font);
	}
	connect(this, SIGNAL(resistanceChanged(double)), this, SLOT(setResistanceInternal(double)));

	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		QHBoxLayout *hLayout = new QHBoxLayout;
		resistanceCheckBox[i] = new QCheckBox(tr("Display Resistance"));
		hLayout->addWidget(resistanceCheckBox[i]);
		hLayout->addStretch(1);
		hLayout->addWidget(resistanceValue[i]);

		resistance[i] = new QGroupBox();
		resistance[i]->setLayout(hLayout);
		connect(resistanceCheckBox[i], SIGNAL(toggled(bool)), this, SLOT(setDisplayOptions()));
	}
}

void ControlWindow::createExternalCommandLayout() {
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		externalCommand[i] = new ExternalCommandWidget(*state.board, i);
	}
}

void ControlWindow::createSignalOutputLayout() {
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		signalOutput[i] = new SignalOutputWidget(*state.board, i, false);
		clampOutput[i] = new SignalOutputWidget(*state.board, i, true);
		markerOutput[i] = new MarkerOutputWidget(*state.board, i);
	}
}

void ControlWindow::measureTemperature() {
    double temperatureC = state.board->controller.temperatureSensor.readTemperature(0);
    double temperatureF = (temperatureC * 1.8) + 32;
    QMessageBox::information(this, "Info", QString("Temperature = ") + QString::number(temperatureC, 'f', 1) + QString(" C") + QString(" = ") + QString::number(temperatureF, 'f', 1) + QString(" F"));
}

void ControlWindow::tryToSetClampMode(int index)
{
    if (enableTabbing) {
        emit clampModeChanged(index);
    }
    else {
        tabWidget[selectedHeadstage()]->setCurrentIndex(currentTabIndex);
    }
}

void ControlWindow::fillSettings(Settings& settings, int unit, bool holdingOnly, unsigned int lastIndex) {
    settings.isVoltageClamp = (tabWidget[unit]->currentIndex() == 0);
	settings.vClampX2mode = state.vClampX2mode;
    appliedVoltageWaveform[unit]->fillWaveformSettings(settings.voltageClamp);
    appliedCurrentWaveform[unit]->fillWaveformSettings(settings.currentClamp);

    settings.pipetteOffset = state.pipetteOffsetInmV[unit];

	settings.Ra = state.datastore[unit].Ra;
	settings.Rm = state.datastore[unit].Rm;
	settings.Cm = state.datastore[unit].Cm;

    settings.waveform = settings.isVoltageClamp ? appliedVoltageWaveform[unit]->getSimplifiedWaveform(state.board->getSamplingRateHz(), holdingOnly, lastIndex) :
                                                  appliedCurrentWaveform[unit]->getSimplifiedWaveform(state.board->getSamplingRateHz(), holdingOnly, lastIndex);
}

void ControlWindow::closeEvent(QCloseEvent *) {
    QWidget* p = dynamic_cast<QWidget*>(parent());
    if (p) {
        p->close();
    }
}

void ControlWindow::setStatusMessage(int unit, QString message) {
    statusLabel[unit]->setText(message);
    statusLabel[unit]->update();
}

int ControlWindow::selectedHeadstage() const {
	return headstageTabWidget->currentIndex();
}

void ControlWindow::setHeadstageFocus(int index)
{
	// state.datastore[index].clear();
	getDisplayWindow()->setUnit(index);
	setDisplayOptions();
}

void ControlWindow::selectFilename()
{
	QString newFileName;
	newFileName = QFileDialog::getSaveFileName(this,
		tr("Select Base Filename"), ".",
		tr("Intan Data Files (*.clp)"));
	if (!newFileName.isEmpty()) {
		saveBaseFileName = newFileName;
		QFileInfo newFileInfo(newFileName);
		saveFilenameLineEdit->setText(newFileInfo.baseName());
	}
	validFilename = !saveBaseFileName.isEmpty();
	recordButton->setEnabled(validFilename);
	getDisplayWindow()->setFilename(saveBaseFileName);
}

void ControlWindow::setSaveAux(bool enable)
{
	state.saveAuxMode = enable;
}

void ControlWindow::setVClampX2(bool x2Mode)
{
	if (x2Mode) {
		int r = QMessageBox::warning(this, tr("CLAMP Controller"),
			tr("<b>Warning:</b> 2x Voltage Clamp Mode is intended only for electrochemical applications such as fast-scan "
				"cyclic voltammetry (FSCV).  All clamp voltages will be 2x the specified values, doubling the voltage clamp "
				"step size to 5 mV and extending the voltage clamp range to +/-1.275 V.<p>"
				"The controls and display will <b>not</b> reflect this 2x increase, "
				"and resistance measurements will be low by a factor of two."
				"<p>Do you want to switch to 2x Voltage Clamp Mode?"),
			QMessageBox::Yes | QMessageBox::No);
		if (r == QMessageBox::No) {
			x2Mode = false;
			vClampX2Action->setChecked(false);
			return;
		}
	}
	state.vClampX2mode = x2Mode;
	for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
		state.board->controller.clampVoltageGenerator.setClampStepSizeImmediate(state.board->getPresentChannels(), x2Mode);
	}
}

// Open Intan Technologies website in the default browser.
void ControlWindow::openIntanWebsite()
{
	QDesktopServices::openUrl(QUrl("http://www.intantech.com", QUrl::TolerantMode));
}

// Display keyboard shortcut window.
void ControlWindow::keyboardShortcutsHelp()
{
	if (!keyboardShortcutDialog) {
		keyboardShortcutDialog = new KeyboardShortcutDialog(this);
	}
	keyboardShortcutDialog->show();
	keyboardShortcutDialog->raise();
	keyboardShortcutDialog->activateWindow();
}

// Display "About" message box.
void ControlWindow::about()
{
	QMessageBox::about(this, tr("About Intan Technologies CLAMP Interface"),
		tr("<h2>Intan Technologies CLAMP Interface</h2>"
			"<p>Version 1.0"
			"<p>Copyright &copy; 2016 Intan Technologies"
			"<p>This biopotential recording application controls the CLAMP "
			"patch clamp interface from Intan Technologies.  The C++/Qt source code "
			"for this application is freely available from Intan Technologies. "
			"For more information visit <i>http://www.intantech.com</i>."
			"<p>This program is free software: you can redistribute it and/or modify "
			"it under the terms of the GNU Lesser General Public License as published "
			"by the Free Software Foundation, either version 3 of the License, or "
			"(at your option) any later version."
			"<p>This program is distributed in the hope that it will be useful, "
			"but WITHOUT ANY WARRANTY; without even the implied warranty of "
			"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
			"GNU Lesser General Public License for more details."
			"<p>You should have received a copy of the GNU Lesser General Public License "
			"along with this program.  If not, see <i>http://www.gnu.org/licenses/</i>."));
}

KeyboardShortcutDialog::KeyboardShortcutDialog(QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Keyboard Shortcuts"));

	QVBoxLayout *controlWindowLayout = new QVBoxLayout;
	controlWindowLayout->addWidget(new QLabel(tr("<b>Mouse Click:</b> Adjust parameter")));
	controlWindowLayout->addWidget(new QLabel(tr("<b>Mouse Wheel:</b> Adjust parameter")));
	controlWindowLayout->addStretch(1);

	QGroupBox *controlWindowGroupBox = new QGroupBox("Control Window");
	controlWindowGroupBox->setLayout(controlWindowLayout);

	QVBoxLayout *displayWindowLayout = new QVBoxLayout;
	displayWindowLayout->addWidget(new QLabel(tr("<b>&lt;/, Key:</b> Zoom in on time axis")));
	displayWindowLayout->addWidget(new QLabel(tr("<b>&gt;/. Key:</b> Zoom out on time axis")));
	displayWindowLayout->addWidget(new QLabel(tr("<b>+/= Key:</b> Zoom in on vertical axis")));
	displayWindowLayout->addWidget(new QLabel(tr("<b>-/_ Key:</b> Zoom out on vertical axis")));
	displayWindowLayout->addWidget(new QLabel(tr("<b>Cursor Keys:</b> Zoom in/out on both axes")));
	displayWindowLayout->addWidget(new QLabel(tr("<b>Page Up/Down Keys:</b> Shift up/down on vertical axis")));
	displayWindowLayout->addWidget(new QLabel(tr("<b>Mouse Click/Drag:</b> (with Autoscale off) Zoom in on axis")));
	displayWindowLayout->addStretch(1);

	QGroupBox *displayWindowGroupBox = new QGroupBox("Data Display Window");
	displayWindowGroupBox->setLayout(displayWindowLayout);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(controlWindowGroupBox);
	mainLayout->addWidget(displayWindowGroupBox);
	mainLayout->addStretch(1);

	setLayout(mainLayout);

}