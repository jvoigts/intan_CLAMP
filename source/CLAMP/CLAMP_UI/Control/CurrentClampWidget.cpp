#include "CurrentClampWidget.h"
#include "WaveformTimingWidget.h"
#include "WaveformAmplitudeWidget.h"
#include "SaveFile.h"
#include <QtGui>
#include "common.h"
#include "GUIUtil.h"
#include "globalconstants.h"
#include "SimplifiedWaveform.h"
#include "AppliedWaveformWidget.h"
#include "DisplayWindow.h"
#include <sstream>
#include "Board.h"
#include "Buzz.h"
#include "PipetteOffset.h"

using namespace CLAMP;
using namespace CLAMP::ClampConfig;
using namespace CLAMP::IO;
using std::ostringstream;
using std::string;

static double steps[] = { 5, 50, 0.5, 1 };
static const char* units[] = { " pA", " pA", " nA", " nA" };
static double magnitudes[] = { 1e-12, 1e-12, 1e-9, 1e-9 };

//--------------------------------------------------------------------------
AppliedCurrentWaveformWidget::AppliedCurrentWaveformWidget() :
    tuningParams(20 * steps[0], steps[0], 127, magnitudes[0], units[0]),
    waveformParams(WaveformAmplitudeParams(steps[0], 127, -20 * steps[0], 5 * steps[0], 20 * steps[0], units[0], magnitudes[0])),
    index(0)
{
    setTitle("Current Clamp Command");

    holding = new QRadioButton(tr("Holding Only"), this);
    tuning = new QRadioButton(tr("Tuning"), this);
    tuningParamsDisplay = new PulseTrainParamsDisplay(tuningParams);

    multistep = new QRadioButton(tr("Multi-Step"), this);
    waveformParamsDisplay = new MultistepParamsDisplay(waveformParams, "current");

	arbWaveform = new QRadioButton(tr("Arbitrary"), this);
	arbWaveform->setEnabled(false);
	arbWaveformParamsDisplay = new ArbWaveformParamsDisplay(true);
	connect(arbWaveformParamsDisplay, SIGNAL(arbWaveformLoaded()), this, SLOT(enableArbWaveformButton()));

    tuning->setChecked(true);

    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(2);
    grid->addWidget(holding, 0, 0, 1, 1);
    grid->addWidget(tuning, 1, 0, 1, 1);
    grid->addWidget(tuningParamsDisplay, 1, 1, 1, 1);
    grid->addWidget(multistep, 2, 0, 1, 1);
    grid->addWidget(waveformParamsDisplay, 2, 1, 1, 1);
	grid->addWidget(arbWaveform, 3, 0, 1, 1);
	grid->addWidget(arbWaveformParamsDisplay, 3, 1, 1, 1);

    setLayout(grid);
}

SimplifiedWaveform AppliedCurrentWaveformWidget::getSimplifiedWaveform(double samplingRate, int holdingValue, bool holdingOnly, unsigned int lastIndex) {
    if (holding->isChecked() || holdingOnly) {
        SimplifiedWaveform simplifiedWaveform;
		if (holdingOnly) {
			simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, lastIndex + 1, 0, false, false));
		}
		else {
			simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, samplingRate, 0, false, false)); // 1 sec worth
		}
        simplifiedWaveform.setStepSize(steps[index] * magnitudes[index], 0);
        return simplifiedWaveform;
    }
    else if (tuning->isChecked()) {
        return tuningParams.getSimplifiedWaveform(samplingRate, holdingValue, 0);
    }
    else if (multistep->isChecked()) {
        return waveformParams.getSimplifiedWaveform(samplingRate, holdingValue, 0);
    }
	else {
		SimplifiedWaveform simplifiedWaveform = arbWaveformParamsDisplay->getSimplifiedWaveform(holdingValue, 0);
		simplifiedWaveform.setStepSize(steps[index] * magnitudes[index], 0);
		return simplifiedWaveform;
	}
}

string AppliedCurrentWaveformWidget::getDescription(const string& holdingValue) const {
    if (holding->isChecked()) {
        ostringstream oss;
        oss << "Running at holding current (" << holdingValue << ")";
        return oss.str();
    }
    else if (tuning->isChecked()) {
        return "Current clamp: Tuning waveform";
    }
    else if (multistep->isChecked()) {
        return "Current clamp: Multi-step waveform";
	}
	else {
		return "Current clamp: Arbitrary waveform";
	}
}

void AppliedCurrentWaveformWidget::setStepIndex(unsigned int index) {
    WaveformAmplitudeParams tmp(steps[index], 127, -20 * steps[index], 5 * steps[index], 20 * steps[index], units[index], magnitudes[index]);
    waveformParams.set(tmp);

    PulseTrainParams tmp2(20 * steps[index], steps[index], 127, magnitudes[index], units[index]);
    tuningParams.set(tmp2);

    this->index = index;
}

void AppliedCurrentWaveformWidget::setEnabled(bool enabled) {
	holding->setEnabled(enabled);
	tuning->setEnabled(enabled);
	multistep->setEnabled(enabled);
	// TODO: also arbWaveform?
}

void AppliedCurrentWaveformWidget::enableArbWaveformButton() {
	arbWaveform->setEnabled(true);
}

//--------------------------------------------------------------------------
CurrentClampWidget::CurrentClampWidget(GlobalState& state_, CapacitiveCompensationController& cap, int unit_) :
    state(state_),
    capController(cap),
	unit(unit_)
{
    applied = new AppliedCurrentWaveformWidget();

    buzz = new BuzzWidget(state, *this, cap, unit_);

    bridgeBalance = new QCheckBox("Bridge Balance", this);
    bridgeBalance->setChecked(false);
    bridgeBalance->setEnabled(true);
    connect(bridgeBalance, SIGNAL(toggled(bool)), this, SLOT(optionsChanged()));

	seriesResistanceSpinBox = GUIUtil::createDoubleSpinBoxZeroFloor(100, 0.1, 0, QString(" M") + QSTRING_OMEGA_SYMBOL);
	seriesResistanceSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
	seriesResistanceSpinBox->setStyleSheet("color:blue; background:transparent; border:none;");
	connect(seriesResistanceSpinBox, SIGNAL(valueChanged(double)), &state.datastore[unit], SLOT(adjustRa(double)));

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(bridgeBalance);
	hLayout->addStretch(1);
	hLayout->addWidget(new QLabel(tr("series resistance:")));
	hLayout->addWidget(seriesResistanceSpinBox);
	hLayout->addStretch(5);
	QGroupBox* bridgeBalanceBox = new QGroupBox();
	bridgeBalanceBox->setLayout(hLayout);

	appliedPlusAdc = new QCheckBox("Plot total Iclamp", this);
	appliedPlusAdc->setChecked(false);
	appliedPlusAdc->setEnabled(true);
	connect(appliedPlusAdc, SIGNAL(toggled(bool)), this, SLOT(optionsChanged()));

	QHBoxLayout *hLayout2 = new QHBoxLayout;
	hLayout2->addWidget(buzz);
	hLayout2->addStretch(1);
	hLayout2->addWidget(appliedPlusAdc);
	hLayout2->addStretch(1);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new PipetteOffsetWidget(state, false, unit));
    layout->addWidget(createHoldingWidget());
    layout->addWidget(applied);
	layout->addWidget(createCurrentScaleWidget());
	layout->addWidget(bridgeBalanceBox);
	layout->addLayout(hLayout2);
    layout->addStretch(1);

    setLayout(layout);

    connect(&state.datastore[unit], SIGNAL(wholeCellParametersChanged(double, double, double)), this, SLOT(setWholeCell(double, double, double)));

    emit setCurrentStepSize(0);
}

QWidget* CurrentClampWidget::createHoldingWidget() {
    holdingAmplitudeSpinBox = GUIUtil::createDoubleSpinBox(127, steps[0], 0, units[0]);
    holdingAmplitudeSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    holdingAmplitudeSpinBox->setStyleSheet("color:blue; background:transparent; border:none;");

    zeroCurrentButton = new QPushButton("Zero Current", this);
    connect(zeroCurrentButton, SIGNAL(clicked()), this, SLOT(setZeroCurrent()));

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(holdingAmplitudeSpinBox);
    hLayout->addStretch(1);
    hLayout->addWidget(zeroCurrentButton);

    QGroupBox* holding = new QGroupBox();
    holding->setTitle("Holding Current");
    holding->setLayout(hLayout);
    return holding;
}

void CurrentClampWidget::setZeroCurrent() {
    ChipChannelList list = { ChipChannel(unit, 0) };
    state.board->controller.switchToCurrentClampImmediate(list, static_cast<CurrentScale>(currentStepSizeComboBox->currentIndex()), 0, capController.getCapCompensationValue());
    state.stateMessage(unit, "Holding at 0 pA");
    holdingAmplitudeSpinBox->setValue(0);
}

QWidget* CurrentClampWidget::createCurrentScaleWidget() {
    currentStepSizeComboBox = new QComboBox();
    currentStepSizeComboBox->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("635 pA in 5 pA steps"));
    currentStepSizeComboBox->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("6.35 nA in 50 pA steps"));
    currentStepSizeComboBox->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("63.5 nA in 0.5 nA steps"));
    currentStepSizeComboBox->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("127 nA in 1 nA steps"));
    currentStepSizeComboBox->setCurrentIndex(0);

    connect(currentStepSizeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentStepSize(int)));

    QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(new QLabel(tr("Current Clamp Range")));
    layout->addWidget(currentStepSizeComboBox);
	layout->addStretch(1);

    QGroupBox* holding = new QGroupBox();
    holding->setLayout(layout);
    return holding;
}

int CurrentClampWidget::getHoldingValue() {
    return lround(holdingAmplitudeSpinBox->value() / holdingAmplitudeSpinBox->singleStep());
}

unsigned int CurrentClampWidget::getCurrentScale() {
    return currentStepSizeComboBox->currentIndex();
}

void CurrentClampWidget::setCurrentStepSize(int index) {
    double stepSize = steps[index];
    holdingAmplitudeSpinBox->setRange(-127 * stepSize, 127 * stepSize);
    holdingAmplitudeSpinBox->setSingleStep(stepSize);
    holdingAmplitudeSpinBox->setSuffix(units[index]);

    applied->setStepIndex(index);
}

void CurrentClampWidget::fillWaveformSettings(CurrentClampSettings& settings) {
    QDoubleSpinBox* spinBox = holdingAmplitudeSpinBox;
    settings.holdingCurrent = spinBox->value() * applied->waveformParams.amplitudeParams.magnitude;
    settings.stepSize = getStepSize(static_cast<CurrentScale>(currentStepSizeComboBox->currentIndex()));
}

SimplifiedWaveform CurrentClampWidget::getSimplifiedWaveform(double samplingRate, bool holdingOnly, unsigned int lastIndex) {
    return applied->getSimplifiedWaveform(samplingRate, getHoldingValue(), holdingOnly, lastIndex);
}

void CurrentClampWidget::startMessage(int unit) {
    state.stateMessage(unit, applied->getDescription(holdingText()).c_str());
}

void CurrentClampWidget::endMessage(int unit) {
    ostringstream oss;
    oss << "Holding at " << holdingText().c_str();
    state.stateMessage(unit, oss.str().c_str());
}

string CurrentClampWidget::holdingText() {
    ostringstream oss;
    oss << holdingAmplitudeSpinBox->value() << units[currentStepSizeComboBox->currentIndex()];
    return oss.str();
}

void CurrentClampWidget::optionsChanged() {
    emit changeDisplay();
}

void CurrentClampWidget::getDisplayConfig(PlotConfiguration& config) {
    config.showVcell = false;
	config.showAppliedPlusAdc = appliedPlusAdc->isChecked();
    config.showBridgeBalance = bridgeBalance->isChecked();
	config.appliedPlot = true;
    config.showMeasuredData = true;
    config.resistancePlot = false;
    config.appliedVoltage = false;
    config.measuredCurrent = false;

    // Possibly could do these
    config.showFittedExponentials = false;
    config.cellParametersCalculation = false;
}

void CurrentClampWidget::setWholeCell(double Ra, double, double) {
    bridgeBalance->setEnabled(true);
	seriesResistanceSpinBox->setValue(Ra / 1e6);
}

void CurrentClampWidget::setControlsEnabled(bool enabled) {
	applied->setEnabled(enabled);
	currentStepSizeComboBox->setEnabled(enabled);
	bridgeBalance->setEnabled(enabled);
	buzz->setControlsEnabled(enabled);
}

