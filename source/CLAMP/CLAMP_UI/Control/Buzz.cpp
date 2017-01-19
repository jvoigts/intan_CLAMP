#include "Buzz.h"
#include <QtGui>
#include "common.h"
#include "GlobalState.h"
#include "globalconstants.h"
#include "ClampController.h"
#include "Board.h"
#include <sstream>
#include "GUIUtil.h"
#include "Controller.h"

using std::string;
using std::ostringstream;
using namespace CLAMP;
using namespace CLAMP::ClampConfig;

//--------------------------------------------------------------------------
BuzzParams::BuzzParams(bool largeAmplitude, double duration) :
    useLarge(largeAmplitude),
    durationMs(duration, 0.1, 0.1, 10)
{
    connect(&useLarge, SIGNAL(valueChanged(bool)), this, SLOT(notifyObserver()));
    connect(&durationMs, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
}

void BuzzParams::notifyObserver() {
    emit valuesChanged();
}

string BuzzParams::toHtml() {
    ostringstream oss;
    oss << "<span style=\"color:blue\">" << (useLarge.value() ? "2X" : "1X") << "</span>, ";
    oss << "<span style=\"color:blue\">" << QString::number(durationMs.value(), 'f', 1).toStdString().c_str() << " ms</span>";
    return oss.str();
}

void BuzzParams::set(const BuzzParams& other) {
    useLarge.set(other.useLarge);
    durationMs.set(other.durationMs);
}

//--------------------------------------------------------------------------
BuzzParamsDisplay::BuzzParamsDisplay(BuzzParams& params_) :
    params(params_) 
{
    QHBoxLayout* layout = new QHBoxLayout();

    label = new QLabel();
    layout->addWidget(label);
    setLayout(layout);

    connect(&params, SIGNAL(valuesChanged()), this, SLOT(redoLayout()));
    redoLayout();
}

void BuzzParamsDisplay::redoLayout() {
    label->setText(params.toHtml().c_str());
}

void BuzzParamsDisplay::mousePressEvent(QMouseEvent *) {
    BuzzParams tmp(true, 0.0);
    tmp.set(params);

    BuzzDialog dialog(this, tmp);
    if (dialog.exec() == QDialog::Accepted) {
        params.set(tmp);
    }
}

//--------------------------------------------------------------------------
BuzzDialog::BuzzDialog(QWidget* parent, BuzzParams& params_) :
    QDialog(parent),
    params(params_)
{
    largeButton = new QRadioButton("2X amplitude", this);
    smallButton = new QRadioButton("1X amplitude", this);
    params.useLarge.connectTo(largeButton);

    if (params.useLarge.value()) {
        largeButton->setChecked(true);
    }
    else {
        smallButton->setChecked(true);
    }

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(smallButton);
    layout->addWidget(largeButton);
    layout->addItem(GUIUtil::createLabeledWidget("Duration:", GUIUtil::createDoubleSpinBox(params.durationMs, " ms")));

    layout->addStretch(1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);

    setLayout(layout);
}

//--------------------------------------------------------------------------
BuzzWidget::BuzzWidget(GlobalState& state_, Controller& controller_, CapacitiveCompensationController& cap_, int unit_) :
    state(state_),
    controller(controller_),
    cap(cap_),
    params(true, 0.5),
    buzzEnabled(false),
	unit(unit_)
{
    button = new QPushButton("Buzz", this);
    connect(button, SIGNAL(clicked()), this, SLOT(doBuzz()));

    paramsDisplay = new BuzzParamsDisplay(params);

    cb = GUIUtil::createLockBox(buzzEnabled);
    QObject::connect(&buzzEnabled, SIGNAL(valueChanged(bool)), button, SLOT(setEnabled(bool)));
    button->setEnabled(buzzEnabled.value());

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(button);
    layout->addWidget(paramsDisplay);
    layout->addWidget(cb);
    setLayout(layout);
}

void BuzzWidget::doBuzz() {
    state.preemptThread(new BuzzThread(state, controller, params, cap, unit));
}

void BuzzWidget::setControlsEnabled(bool enabled) {
	if (!enabled) {
		button->setEnabled(false);
		cb->setEnabled(false);
	}
	else {
		button->setEnabled(buzzEnabled.value());
		cb->setEnabled(true);
	}
}

//--------------------------------------------------------------------------
BuzzThread::BuzzThread(GlobalState& state_, Controller& controller_, BuzzParams& params_, CapacitiveCompensationController& cap_, int unit_) :
    Thread(),
    state(state_),
    controller(controller_),
    cap(cap_),
    params(params_),
    sound(new QSound(QDir::tempPath() + "/beep.wav")),
	unit(unit_)
{
}

BuzzThread::~BuzzThread() {

}

void BuzzThread::done() {
    state.threadDone();
}

void BuzzThread::run() {
    state.stateMessage(unit, "Buzz");

    sound->play();

	state.board->enableOnePortOnly(unit);

    ChipChannelList channelList = { ChipChannel(unit, 0) };
    state.board->controller.switchToCurrentClampImmediate(channelList, static_cast<CurrentScale>(controller.getCurrentScale()), controller.getHoldingValue(), cap.getCapCompensationValue());
    state.board->controller.fastTransientCapacitiveCompensation.buzzImmediate(channelList, params.useLarge.value(), params.durationMs.value() / 1000);
    state.board->controller.clampCurrentGenerator.setCurrentImmediate(channelList, controller.getHoldingValue());

	state.board->enableAllPorts();

    controller.endMessage(unit);
}

