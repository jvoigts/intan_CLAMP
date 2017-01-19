#include "FeedbackBandwidthWidget.h"
#include <QtGui>
#include "common.h"
#include "SaveFile.h"
#include "globalconstants.h"
#include "Channel.h"

using namespace CLAMP;
using CLAMP::Registers::Register3;

//--------------------------------------------------------------------------
FeedbackBandwidthWidget::FeedbackBandwidthWidget(CLAMP::Channel& channel_) :
    channel(channel_),
    r3(channel_.registers.r3.value),
    r4(channel_.registers.r4.value)
{
    currentScale = new QComboBox();
    currentScale->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("1000 nA"));
    currentScale->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("100 nA"));
    currentScale->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("10 nA"));
    currentScale->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("5 nA"));
	// currentScale->addItem(QSTRING_PLUSMINUS_SYMBOL + tr("2.5 nA"));  // NOTE: This range offers no improvement in noise floor, so we don't use it.
	connect(currentScale, SIGNAL(currentIndexChanged(int)), this, SLOT(setResistor(int)));

    //measuredResistance = new QLabel(this);
    //minBandwidth = new QLabel(this);
    //maxBandwidth = new QLabel(this);
    //actualBandwidth = new QLabel(this);
    //feedbackCapacitor = new QLabel(this);

    //desiredBandwidth = new QDoubleSpinBox();
    //desiredBandwidth->setRange(1, 2500);
    //desiredBandwidth->setSingleStep(1);
    //desiredBandwidth->setSuffix(" kHz");
    //desiredBandwidth->setValue(channel.desiredBandwidth / 1000.0);
    //desiredBandwidth->setDecimals(0);
    //desiredBandwidth->setKeyboardTracking(false);
    //connect(desiredBandwidth, SIGNAL(valueChanged(double)), this, SLOT(setDesiredBandwidth()));

    QHBoxLayout *hLayout1 = new QHBoxLayout();
    hLayout1->addWidget(new QLabel("Current Measurement Range"));
    hLayout1->addStretch(1);
    hLayout1->addWidget(currentScale);
    hLayout1->addStretch(10);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addItem(hLayout1);
    layout->addStretch(1);

    setLayout(layout);
    // setTitle(tr("Measurement Range"));

    currentScale->setCurrentIndex(3);
	setResistor(currentScale->currentIndex());
}

void FeedbackBandwidthWidget::setResistor(int value) {
    setResistorInternal(static_cast<Register3::Resistance>(value + 1));
	emit feedbackResistanceChanged(value);
    //double rF = getResistance();
    //switch (value) {
    //case 0:
    //    measuredResistance->setText(QString::number(rF / 1e3, 'f', 0) + " k" + QSTRING_OMEGA_SYMBOL);
    //    break;
    //case 1:
    //    measuredResistance->setText(QString::number(rF / 1e6, 'f', 2) + " M" + QSTRING_OMEGA_SYMBOL);
    //    break;
    //case 2:
    //case 3:
	//case 4:
    //    measuredResistance->setText(QString::number(rF / 1e6, 'f', 1) + " M" + QSTRING_OMEGA_SYMBOL);
    //    break;
    //}
    //measuredResistance->update();

    //double minValue = BandwidthHelper::getBandwidth(rF, 255 * 0.2e-12);
    //double maxValue = BandwidthHelper::getBandwidth(rF, 1 * 0.2e-12);

    //minBandwidth->setText(getBandwidthString(minValue) + " <= ");
    //minBandwidth->update();
    //maxBandwidth->setText(tr("<= ") + getBandwidthString(maxValue));
    //maxBandwidth->update();

    bandwidthChanged();
}

void FeedbackBandwidthWidget::setDesiredBandwidth() {
    setBestCapacitor();
}

void FeedbackBandwidthWidget::bandwidthChanged() {
    //actualBandwidth->setText(getBandwidthString(getActualBandwidth()));
    //actualBandwidth->update();
    //feedbackCapacitor->setText(QString::number(getCapacitance() * 1e12, 'f', 1) + " pF");
    //feedbackCapacitor->update();
}

QString FeedbackBandwidthWidget::getBandwidthString(double value) {
    if (value < 1e3) {
        return QString::number(value, 'f', 0) + " Hz";
    }
    else if (value < 1e6) {
        return QString::number(value / 1e3, 'f', 0) + " kHz";
    }
    else {
        return QString::number(value / 1e6, 'f', 2) + " MHz";
    }
}

void FeedbackBandwidthWidget::setResistorInternal(Register3::Resistance value) {
    r3.feedbackResistance = value;
    setBestCapacitor();
}

double FeedbackBandwidthWidget::getResistance() {
    return channel.rFeedback[getResistanceEnum()];
}

double FeedbackBandwidthWidget::getCapacitance() {
    return r4.capacitance();
}

double FeedbackBandwidthWidget::getActualBandwidth() {
    double r = getResistance();
    double c = getCapacitance();
    return BandwidthHelper::getBandwidth(r, c);
}

void FeedbackBandwidthWidget::setBestCapacitor() {
    double r = getResistance();
    double desiredC = BandwidthHelper::getC(r, getDesiredBandwidth());
    r4.setFeedbackCapacitance(desiredC);
    bandwidthChanged();
}

double FeedbackBandwidthWidget::getDesiredBandwidth() const {
    return 10000;
    //return desiredBandwidth->value() * 1000.0;
}

Register3::Resistance FeedbackBandwidthWidget::getResistanceEnum() {
    return r3.resistanceEnum();
}

bool FeedbackBandwidthWidget::valuesChanged() {
    if (channel.desiredBandwidth != getDesiredBandwidth()) {
        return true;
    }
    if (channel.registers.r3.value != r3) {
        return true;
    }
    if (channel.registers.r4.value != r4) {
        return true;
    }
    return false;
}

void FeedbackBandwidthWidget::setControlsEnabled(bool enabled) {
	currentScale->setEnabled(enabled);
}