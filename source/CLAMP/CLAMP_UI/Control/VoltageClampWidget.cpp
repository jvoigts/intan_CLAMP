#include "VoltageClampWidget.h"
#include "WaveformTimingWidget.h"
#include "WaveformAmplitudeWidget.h"
#include "SaveFile.h"
#include <QtGui>
#include "common.h"
#include "GUIUtil.h"
#include "GlobalState.h"
#include "AppliedWaveformWidget.h"
#include "HoldingVoltageWidget.h"
#include <sstream>
#include "FeedbackBandwidthWidget.h"
#include "Board.h"
#include "DisplayWindow.h"
#include "globalconstants.h"
#include "Zap.h"
#include "PipetteOffset.h"

using namespace CLAMP;
using namespace CLAMP::IO;
using std::ostringstream;
using std::string;

//--------------------------------------------------------------------------
AppliedVoltageWaveformWidget::AppliedVoltageWaveformWidget() :
    sealTestParams(10.0, 2.5, 255, 1e-3, " mV"),
    multistepParams(WaveformAmplitudeParams(2.5, 255, 10, 10, 30, " mV", 1e-3))
{
    setTitle("Voltage Clamp Command");

    holding = new QRadioButton(tr("Holding Only"), this);
    sealTest = new QRadioButton(tr("Seal Test"), this);
    sealTestParamsDisplay = new PulseTrainParamsDisplay(sealTestParams);

    resistance = new QRadioButton(tr("Resistance"), this);
    resistanceParamsDisplay = new ResistanceParamsDisplay(resistanceParams);

    multistep = new QRadioButton(tr("Multi-Step"), this);
    multistepParamsDisplay = new MultistepParamsDisplay(multistepParams, "voltage");

	arbWaveform = new QRadioButton(tr("Arbitrary"), this);
	arbWaveform->setEnabled(false);
	arbWaveformParamsDisplay = new ArbWaveformParamsDisplay(false);
	connect(arbWaveformParamsDisplay, SIGNAL(arbWaveformLoaded()), this, SLOT(enableArbWaveformButton()));

    sealTest->setChecked(true);  // TODO: set back to 'holding'?

    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(2);
    grid->addWidget(holding, 0, 0, 1, 1);
    grid->addWidget(sealTest, 1, 0, 1, 1);
    grid->addWidget(sealTestParamsDisplay, 1, 1, 1, 1);
    grid->addWidget(resistance, 2, 0, 1, 1);
    grid->addWidget(resistanceParamsDisplay, 2, 1, 1, 1);
    grid->addWidget(multistep, 3, 0, 1, 1);
    grid->addWidget(multistepParamsDisplay, 3, 1, 1, 1);
	grid->addWidget(arbWaveform, 4, 0, 1, 1);
	grid->addWidget(arbWaveformParamsDisplay, 4, 1, 1, 1);

    setLayout(grid);
}

void AppliedVoltageWaveformWidget::enableControls(bool enabled) {
	holding->setEnabled(enabled);
	sealTest->setEnabled(enabled);
	resistance->setEnabled(enabled);
	multistep->setEnabled(enabled);
	// TODO: arbWaveform, too?  However, this method doesn't seem to be used anywhere...
}

SimplifiedWaveform AppliedVoltageWaveformWidget::getSimplifiedWaveform(double samplingRate, int holdingValue, double pipetteOffset, bool holdingOnly, unsigned int lastIndex) {
    if (holding->isChecked() || holdingOnly) {
        SimplifiedWaveform simplifiedWaveform;
		if (holdingOnly) {
			simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, lastIndex + 1, 0, false, false));
		}
		else {
			simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, samplingRate, 0, false, false)); // 1 sec worth
		}
        simplifiedWaveform.setStepSize(2.5e-3, pipetteOffset);
        return simplifiedWaveform;

    } else if (sealTest->isChecked()) {
        return sealTestParams.getSimplifiedWaveform(samplingRate, holdingValue, pipetteOffset);
    }
    else if (resistance->isChecked()) {
        return resistanceParams.getSimplifiedWaveform(samplingRate, holdingValue, pipetteOffset);
    }
    else if (multistep->isChecked()) {
        return multistepParams.getSimplifiedWaveform(samplingRate, holdingValue, pipetteOffset);
    }
	else {
		SimplifiedWaveform simplifiedWaveform = arbWaveformParamsDisplay->getSimplifiedWaveform(holdingValue, pipetteOffset);
		simplifiedWaveform.setStepSize(2.5e-3, pipetteOffset);
		return simplifiedWaveform;
	}
}

string AppliedVoltageWaveformWidget::getDescription(const string& holdingString) const {
    if (holding->isChecked()) {
        ostringstream oss;
        oss << "Running at holding voltage (" << holdingString << ")";
        return oss.str();
    }
    else if (sealTest->isChecked()) {
        return "Voltage Clamp: Seal test";
    }
    else if (resistance->isChecked()) {
        return "Repeating resistance measurement;\nHolding voltage in between";
    }
    else if (multistep->isChecked()) {
        return "Voltage Clamp: Multi-step waveform";
    }
	else {
		return "Voltage Clamp: Arbitrary waveform";
	}
}

void AppliedVoltageWaveformWidget::enableArbWaveformButton() {
	arbWaveform->setEnabled(true);
}

//--------------------------------------------------------------------------
VoltageClampWidget::VoltageClampWidget(GlobalState& state_, int unit_) :
    state(state_),
	unit(unit_)
{
    holdingVoltage = new HoldingVoltageWidget();
    applied = new AppliedVoltageWaveformWidget();
    feedback = new FeedbackBandwidthWidget(*state.board->chip[unit]->channel[0]);

    createCellParametersLayout();

    vCellCorrection = new QCheckBox("Plot Vcell", this);
    vCellCorrection->setChecked(false);
    vCellCorrection->setEnabled(false);
    connect(vCellCorrection, SIGNAL(toggled(bool)), this, SLOT(optionsChanged()));

	appliedPlusAdc = new QCheckBox("Plot total Vclamp", this);
	appliedPlusAdc->setChecked(false);
	appliedPlusAdc->setEnabled(true);
	connect(appliedPlusAdc, SIGNAL(toggled(bool)), this, SLOT(optionsChanged()));

    zap = new ZapWidget(state, *this, *feedback, unit);

	QHBoxLayout *hlayout1 = new QHBoxLayout;
	hlayout1->addStretch(1);
	hlayout1->addWidget(appliedPlusAdc);
	hlayout1->addStretch(1);
	hlayout1->addWidget(vCellCorrection);

	QVBoxLayout *vlayout = new QVBoxLayout;
	vlayout->addStretch(1);
	vlayout->addWidget(zap);
	vlayout->addStretch(1);
	vlayout->addItem(hlayout1);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(cellParameters);
    hlayout->addStretch(1);
	hlayout->addLayout(vlayout);

    QVBoxLayout *layout = new QVBoxLayout;
    pipetteOffset = new PipetteOffsetWidget(state, true, unit, this, feedback);
    layout->addWidget(pipetteOffset);
    layout->addWidget(holdingVoltage);
    layout->addWidget(applied);
    layout->addWidget(feedback);
    layout->addItem(hlayout);
    layout->addStretch(1);

    setLayout(layout);

    connect(applied->resistance, SIGNAL(toggled(bool)), this, SLOT(optionsChanged()));
}

void VoltageClampWidget::createCellParametersLayout() {
    raValue = new QLabel(tr("Rs = -.- M") + QSTRING_OMEGA_SYMBOL, this);
    rmValue = new QLabel(tr("Rm = -.- M") + QSTRING_OMEGA_SYMBOL, this);
    cmValue = new QLabel(tr("Cm = -.- pF"), this);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(raValue);
    layout->addWidget(rmValue);
    layout->addWidget(cmValue);

    cellParameters = new QGroupBox();
    cellParameters->setLayout(layout);
    cellParameters->setTitle("Cell Parameters");
    cellParameters->setCheckable(true);
    cellParameters->setChecked(false);
    connect(cellParameters, SIGNAL(toggled(bool)), this, SLOT(optionsChanged()));

    state.datastore[unit].cellParametersValue.connectTo(cellParameters);
    connect(&state.datastore[unit], SIGNAL(wholeCellParametersChanged(double, double, double)), this, SLOT(setWholeCell(double, double, double)));
}

int VoltageClampWidget::getHoldingValue() {
    return holdingVoltage->valueInSteps(state.pipetteOffsetInmV[unit]);
}

void VoltageClampWidget::fillWaveformSettings(VoltageClampSettings& settings) {
    settings.holdingVoltage = holdingVoltage->valuemV() * 1e-3;
}

SimplifiedWaveform VoltageClampWidget::getSimplifiedWaveform(double samplingRate, bool holdingOnly, unsigned int lastIndex) {
    return applied->getSimplifiedWaveform(samplingRate, getHoldingValue(), state.pipetteOffsetInmV[unit] * 1e-3, holdingOnly, lastIndex);
}

void VoltageClampWidget::startMessage(int unit) {
    state.stateMessage(unit, applied->getDescription(holdingText()).c_str());
}

void VoltageClampWidget::endMessage(int unit) {
    ostringstream oss;
    oss << "Holding at " << holdingText().c_str();
    state.stateMessage(unit, oss.str().c_str());
}

string VoltageClampWidget::holdingText() {
    ostringstream oss;
    oss << holdingVoltage->valuemV() << " mV";
    return oss.str();
}

void VoltageClampWidget::getDisplayConfig(PlotConfiguration& config) {
    config.showVcell = vCellCorrection->isChecked();
	config.showAppliedPlusAdc = appliedPlusAdc->isChecked();
    config.showBridgeBalance = false;
    config.appliedPlot = true;
    config.showMeasuredData = true;
    config.showFittedExponentials = cellParameters->isChecked();
    config.cellParametersCalculation = config.showFittedExponentials;
    config.appliedVoltage = true;
    config.measuredCurrent = true;

    config.resistancePlot = applied->resistance->isChecked();
}

void VoltageClampWidget::optionsChanged() {
    emit changeDisplay();
}

void VoltageClampWidget::setWholeCell(double Ra, double Rm, double Cm) {
    raValue->setText(tr("Rs = ") + QString::number(Ra / 1e6, 'f', 1) + " M" + QSTRING_OMEGA_SYMBOL);
    rmValue->setText(tr("Rm = ") + QString::number(Rm / 1e6, 'f', 1) + " M" + QSTRING_OMEGA_SYMBOL);
    cmValue->setText(tr("Cm = ") + QString::number(Cm / 1e-12, 'f', 1) + " pF");
    raValue->update();
    rmValue->update();
    cmValue->update();

    vCellCorrection->setEnabled(true);
}

void VoltageClampWidget::setControlsEnabled(bool enabled) {
	holdingVoltage->setEnabled(enabled);
	applied->setEnabled(enabled);
	feedback->setEnabled(enabled);
	vCellCorrection->setEnabled(enabled);
	cellParameters->setEnabled(enabled);
	zap->setControlsEnabled(enabled);
}
