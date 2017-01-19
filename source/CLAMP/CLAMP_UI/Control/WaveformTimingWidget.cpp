#include "WaveformTimingWidget.h"
#include <QtGui>
#include "common.h"
#include "GUIUtil.h"
#include <sstream>

using std::ostringstream;
using std::string;

//--------------------------------------------------------------------------
TimingParams::TimingParams() :
    useFrequency(true),
    frequency(50.0),
    holdTimeMs(50),
    pulseTimeMs(30)
{

    connect(&useFrequency, SIGNAL(valueChanged(bool)), this, SLOT(notifyObserver()));
    connect(&frequency, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
    connect(&pulseTimeMs, SIGNAL(valueChanged(int)), this, SLOT(notifyObserver()));
    connect(&holdTimeMs, SIGNAL(valueChanged(int)), this, SLOT(notifyObserver()));
}

void TimingParams::getNumReps(double samplingFreq, unsigned int& numRepsHold, unsigned int& numRepsStep) {
    if (useFrequency.value()) {
        int cyclesPerPeriod = round(samplingFreq / frequency.value());
        numRepsHold = cyclesPerPeriod / 2;
        numRepsStep = cyclesPerPeriod - numRepsHold;
    }
    else {
        numRepsHold = round(samplingFreq * holdTimeMs.value() / 1000.0);
        numRepsStep = round(samplingFreq * pulseTimeMs.value() / 1000.0);
    }
}

string TimingParams::toHtml() {
    ostringstream oss;
    if (useFrequency.value()) {
        oss << " @ <span style=\"color:blue\">" << QString::number(frequency.value(), 'f', 0).toStdString().c_str() << " Hz</span>";
    }
    else {
        oss << " (<span style=\"color:blue\">" << QString::number(pulseTimeMs.value(), 'f', 0).toStdString().c_str() << " ms</span> on,"
            << " <span style=\"color:blue\">" << QString::number(holdTimeMs.value(), 'f', 0).toStdString().c_str() << " ms</span> off)";
    }
    return oss.str();
}

void TimingParams::notifyObserver() {
    emit valuesChanged();
}

void TimingParams::set(const TimingParams& other) {
    useFrequency.set(other.useFrequency);
    frequency.set(other.frequency);
    holdTimeMs.set(other.holdTimeMs);
    pulseTimeMs.set(other.pulseTimeMs);
}

//--------------------------------------------------------------------------
WaveformTimingWidget::WaveformTimingWidget(TimingParams& params_) :
    params(params_)
{
    useFreqRadioButton = new QRadioButton(tr("Frequency"));
    useDurationRadioButton = new QRadioButton(tr("Duration"));

    frequencySpinBox = new QDoubleSpinBox();
    frequencySpinBox->setRange(1, 1000);
    frequencySpinBox->setSingleStep(1);
    frequencySpinBox->setSuffix(" Hz");
    frequencySpinBox->setValue(params.frequency.value());
    frequencySpinBox->setDecimals(0);
    frequencySpinBox->setKeyboardTracking(false);

    pulseDurationSpinBox = GUIUtil::createDurationSpinBox(params.pulseTimeMs.value());

    holdDurationSpinBox = GUIUtil::createDurationSpinBox(params.holdTimeMs.value());

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(useFreqRadioButton);
    leftLayout->addSpacing(10);
    leftLayout->addItem(GUIUtil::createLabeledWidget("Frequency:", frequencySpinBox));
    leftLayout->addStretch(1);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(useDurationRadioButton);
    rightLayout->addSpacing(10);
    rightLayout->addItem(GUIUtil::createLabeledWidget("Pulse duration:", pulseDurationSpinBox));
    rightLayout->addItem(GUIUtil::createLabeledWidget("Hold duration:", holdDurationSpinBox));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addItem(leftLayout);
    layout->addSpacing(10);
    layout->addItem(rightLayout);

    useFreqRadioButton->setChecked(params.useFrequency.value());
    useDurationRadioButton->setChecked(!params.useFrequency.value());
    emit setWaveformTimingType();

    setTitle(tr("Timing"));
    setLayout(layout);

    params.useFrequency.connectTo(useFreqRadioButton);
    QObject::connect(&params.useFrequency, SIGNAL(valueChanged(bool)), this, SLOT(setWaveformTimingType()));

    params.frequency.connectTo(frequencySpinBox);
    params.pulseTimeMs.connectTo(pulseDurationSpinBox);
    params.holdTimeMs.connectTo(holdDurationSpinBox);
}

void WaveformTimingWidget::setWaveformTimingType() {
    bool useFreq = params.useFrequency.value();
    frequencySpinBox->setEnabled(useFreq);
    pulseDurationSpinBox->setEnabled(!useFreq);
    holdDurationSpinBox->setEnabled(!useFreq);
}
