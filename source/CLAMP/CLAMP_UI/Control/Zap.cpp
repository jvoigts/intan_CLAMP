#include "Zap.h"
#include <QtGui>
#include "common.h"
#include "GlobalState.h"
#include "globalconstants.h"
#include "ClampController.h"
#include "Board.h"
#include <sstream>
#include "GUIUtil.h"
#include "Controller.h"
#include "FeedbackBandwidthWidget.h"
#include "ControlWindow.h"

using std::string;
using std::ostringstream;
using namespace CLAMP;
using namespace CLAMP::ClampConfig;

//--------------------------------------------------------------------------
ZapParams::ZapParams(double amplitude, double duration) :
    amplitudeV(amplitude, 0.1, -255 * 5e-3, 255 * 5e-3),
    durationMs(duration, 0.1, 0.1, 10)
{
    connect(&amplitudeV, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
    connect(&durationMs, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
}

SimplifiedWaveform ZapParams::getSimplifiedWaveform(double samplingRate, int holdingValue) {
    SimplifiedWaveform waveform;

    // Zap voltage (e.g., 1 V, 500 us)
    int value = amplitudeV.value() / 5e-3;
    unsigned int numTimesteps = samplingRate * (durationMs.value() / 1000);
    waveform.push_back(WaveformSegment(0, value, numTimesteps, 0, true, true));

    // Holding (e.g., 0 V, 1 timestep)
    waveform.push_back(WaveformSegment(0, holdingValue, 1, 0, false, false));
    return waveform;
}

void ZapParams::notifyObserver() {
    emit valuesChanged();
}

string ZapParams::toHtml() {
    ostringstream oss;
    oss << "<span style=\"color:blue\">" << amplitudeV.value() << " V</span>, ";
    oss << "<span style=\"color:blue\">" << QString::number(durationMs.value(), 'f', 1).toStdString().c_str() << " ms</span>";
    return oss.str();
}

void ZapParams::set(const ZapParams& other) {
    amplitudeV.set(other.amplitudeV);
    durationMs.set(other.durationMs);
}

//--------------------------------------------------------------------------
ZapParamsDisplay::ZapParamsDisplay(ZapParams& params_) :
    params(params_) 
{
    QHBoxLayout* layout = new QHBoxLayout();

    label = new QLabel();
    layout->addWidget(label);
    setLayout(layout);

    connect(&params, SIGNAL(valuesChanged()), this, SLOT(redoLayout()));
    redoLayout();
}

void ZapParamsDisplay::redoLayout() {
    label->setText(params.toHtml().c_str());
}

void ZapParamsDisplay::mousePressEvent(QMouseEvent *) {
    ZapParams tmp(0.0, 0.0);
    tmp.set(params);

    ZapDialog dialog(this, tmp);
    if (dialog.exec() == QDialog::Accepted) {
        params.set(tmp);
    }
}

//--------------------------------------------------------------------------
ZapDialog::ZapDialog(QWidget* parent, ZapParams& params_) :
    QDialog(parent),
    params(params_)
{

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addItem(GUIUtil::createLabeledWidget("Amplitude:", GUIUtil::createDoubleSpinBox(params.amplitudeV, " V")));
    layout->addItem(GUIUtil::createLabeledWidget("Duration:", GUIUtil::createDoubleSpinBox(params.durationMs, " ms")));

    layout->addStretch(1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);

    setLayout(layout);
}

//--------------------------------------------------------------------------
ZapWidget::ZapWidget(GlobalState& state_, Controller& controller_, FeedbackBandwidthWidget& feedback_, int unit_) :
    state(state_),
    controller(controller_),
    feedback(feedback_),
    params(1.0, 0.5),
    zapEnabled(false),
	unit(unit_)
{
    button = new QPushButton("Zap", this);
    connect(button, SIGNAL(clicked()), this, SLOT(doZap()));

    paramsDisplay = new ZapParamsDisplay(params);

    cb = GUIUtil::createLockBox(zapEnabled);
    QObject::connect(&zapEnabled, SIGNAL(valueChanged(bool)), button, SLOT(setEnabled(bool)));
    button->setEnabled(zapEnabled.value());

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(button);
    layout->addWidget(paramsDisplay);
    layout->addWidget(cb);
    setLayout(layout);
}

void ZapWidget::doZap() {
    state.preemptThread(new ZapThread(state, params, controller, feedback, unit));
}

void ZapWidget::setControlsEnabled(bool enabled) {
	if (!enabled) {
		button->setEnabled(false);
	}
	else {
		button->setEnabled(zapEnabled.value());
	}
	cb->setEnabled(enabled);
}

//--------------------------------------------------------------------------
ZapThread::ZapThread(GlobalState& state_, ZapParams& params_, Controller& controller_, FeedbackBandwidthWidget& feedback_, int unit_) :
    Thread(),
    state(state_),
    params(params_),
    controller(controller_),
    feedback(feedback_),
    beep(new QSound(QDir::tempPath() + "/beep.wav")),
	unit(unit_)
{
}

ZapThread::~ZapThread() {

}

void ZapThread::done() {
    state.threadDone();
}

void ZapThread::run() {
    state.stateMessage(unit, "Zap");

    beep->play();

	state.board->enableOnePortOnly(unit);

	// TODO: What about other channels?
    ChipChannelList channelList = { ChipChannel(unit, 0) };
    CapacitiveCompensationController& cap = *state.datastore[unit].controlWindow;
    state.board->controller.switchToVoltageClampImmediate(channelList, controller.getHoldingValue(), feedback.getDesiredBandwidth(), feedback.getResistanceEnum(), cap.getCapCompensationValue());

	// Switch to smallest feedback resistor to allow large currents to be delivered.
	state.board->controller.currentToVoltageConverter.setFeedbackResistanceImmediate(channelList, CLAMP::Registers::Register3::Resistance::R200k);
	state.board->controller.currentToVoltageConverter.setFeedbackCapacitanceImmediate(channelList, 20e-12);

    SimplifiedWaveform waveform = params.getSimplifiedWaveform(state.board->getSamplingRateHz(), controller.getHoldingValue() / 2); // /2 is because we're using 5 mV steps

    state.board->controller.clampVoltageGenerator.setClampStepSize(channelList.front(), true); // set voltage clamp DAC step size to 5.0 mV to accommodate large zap pulse
    state.board->controller.simplifiedWaveformToWaveform(channelList, true, waveform);
    state.board->controller.executeImmediate(channelList);
	state.board->controller.clampVoltageGenerator.setClampStepSize(channelList.front(), state.vClampX2mode); // set voltage clamp DAC step size for normal operation

    // Return to exact holding value - we may have been off by 1 step due to 5 mV steps instead of 2.5 mV steps.  Also, restore old value of feedback resistor.
    // state.board->controller.clampVoltageGenerator.setClampVoltageImmediate(channelList, controller.getHoldingValue());
	state.board->controller.switchToVoltageClampImmediate(channelList, controller.getHoldingValue(), feedback.getDesiredBandwidth(), feedback.getResistanceEnum(), cap.getCapCompensationValue());

	state.board->enableAllPorts();

    controller.endMessage(unit);
}

