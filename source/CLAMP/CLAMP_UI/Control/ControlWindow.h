#pragma once

#include <QMainWindow>
#include "Constants.h"
#include "Controller.h"
#include "ClampThread.h"

class VoltageClampWidget;
class CurrentClampWidget;
class ExternalCommandWidget;
class SignalOutputWidget;
class MarkerOutputWidget;
class QLayoutItem;
class QLabel;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QDoubleSpinBox;
class QTabWidget;
class QAction;
class QGroupBox;
class GlobalState;
class DisplayWindow;
class KeyboardShortcutDialog;

namespace CLAMP {
	namespace IO {
		struct Settings;
	}
}

class ControlWindow : public QMainWindow, public CapacitiveCompensationController {
	Q_OBJECT

public:
	ControlWindow(QWidget* parent, GlobalState& state_);
	~ControlWindow();

	double getCapCompensationValue() const;
	void setResistance(double value);
	void updateStatsExt();
	void fillSettings(CLAMP::IO::Settings& settings, int unit, bool holdingOnly = false, unsigned int lastIndex = 0);

signals:
	void updateStatsSignal();
	void resistanceChanged(double value);
	void clampModeChanged(int index);

	public slots:
	void setDisplayOptions();

	private slots:
	void setResistanceInternal(double value);
	void updateStats();
	void setThreadStatus(bool running);
	void setClampMode(int index);
	void tryToSetClampMode(int index);
	void setHeadstageFocus(int index);
	void setCapacitiveCompensation();
	void enableCapacitiveCompensation(bool checked);
	void selectFilename();
	void startRecording();
	void startRunning();
	void startRunningOnce();
	void stopRunning();
	void measureTemperature();
	void setSaveAux(bool enable);
	void setVClampX2(bool x2Mode);
	void openIntanWebsite();
	void keyboardShortcutsHelp();
	void about();
	void setStatusMessage(int unit, QString message); // Note: should not be QString&

	// Should be private some day
public:
	// Headstage selection tabs
	QTabWidget* headstageTabWidget;

	// Applied Waveform specification
	QTabWidget* tabWidget[CLAMP::MAX_NUM_CHIPS];
	VoltageClampWidget* appliedVoltageWaveform[CLAMP::MAX_NUM_CHIPS];
	CurrentClampWidget* appliedCurrentWaveform[CLAMP::MAX_NUM_CHIPS];

private:
	GlobalState& state;

	void createActions();
	void createMenus();
	void createStatusBar();
	void createLayout();

	// Menus
	QAction* measureTemperatureAction;
	QAction *keyboardHelpAction;
	QAction* intanWebsiteAction;
	QAction* aboutAction;
	QAction* saveAuxAction;
	QAction* vClampX2Action;

	QLayout* createControlLayout();

	// Run/Stop & label
	QLayoutItem* createRunStopLayout();
	QLayoutItem* createFIFOStatsLayout();
	QLabel *fifoLagLabel;
	QLabel *fifoFullLabel;
	QPushButton* runButton;
	QPushButton* runOnceButton;
	QPushButton* stopButton;
	QPushButton* recordButton;
	QPushButton* selectFilenameButton;
	QLineEdit* saveFilenameLineEdit;
	QString saveBaseFileName;
	bool validFilename;

	void startRunningWithType(ClampThread::RunType runtype);

	// Capacitive compensation
	QGroupBox* capCompensation[CLAMP::MAX_NUM_CHIPS];
	void createCapCompensation();
	QDoubleSpinBox* capCompensationSpinBox[CLAMP::MAX_NUM_CHIPS];

	// What's happening
	QLabel* statusLabel[CLAMP::MAX_NUM_CHIPS];
	QWidget* createStateBox(int unit);

	// Applied Waveform specification
	bool enableTabbing;
	int currentTabIndex;

	// Resistance
	QGroupBox* resistance[CLAMP::MAX_NUM_CHIPS];
	QCheckBox* resistanceCheckBox[CLAMP::MAX_NUM_CHIPS];
	QLabel* resistanceValue[CLAMP::MAX_NUM_CHIPS];
	void createResistanceLayout();

	// External commands from ANALOG IN ports
	ExternalCommandWidget* externalCommand[CLAMP::MAX_NUM_CHIPS];
	void createExternalCommandLayout();

	// Signal outputs to ANALOG OUT and DIGITAL OUT ports
	SignalOutputWidget* signalOutput[CLAMP::MAX_NUM_CHIPS];
	SignalOutputWidget* clampOutput[CLAMP::MAX_NUM_CHIPS];
	MarkerOutputWidget* markerOutput[CLAMP::MAX_NUM_CHIPS];
	void createSignalOutputLayout();

	KeyboardShortcutDialog *keyboardShortcutDialog;

	void closeEvent(QCloseEvent *e) override;
	DisplayWindow* getDisplayWindow() const;
	int selectedHeadstage() const;
	int runningHeadstage;
};

class KeyboardShortcutDialog : public QDialog
{
	Q_OBJECT
public:
	explicit KeyboardShortcutDialog(QWidget *parent = 0);

signals:

	public slots :

};
