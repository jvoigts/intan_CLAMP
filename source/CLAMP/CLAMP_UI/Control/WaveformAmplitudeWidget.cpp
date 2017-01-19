#include "WaveformAmplitudeWidget.h"
#include <QtGui>
#include "common.h"
#include "GUIUtil.h"
#include <sstream>

using std::string;
using std::ostringstream;

//--------------------------------------------------------------------------
WaveformAmplitudeParams::WaveformAmplitudeParams(double stepSize_, int maxValue_, double defaultStart, double defaultStep, double defaultEnd, const char* units_, double magnitude_) :
    QObject(),
    startAmplitude(defaultStart, stepSize_, -maxValue_ * stepSize_, maxValue_ * stepSize_),
    stepSize(defaultStep, stepSize_, -2 * maxValue_ * stepSize_, 2 * maxValue_ * stepSize_),  // E.g., if the range of applied values is -100 to +100, we should be able to do a +200 step here
    endAmplitude(defaultEnd, stepSize_, -maxValue_ * stepSize_, maxValue_ * stepSize_),
    multiStep(true),
    returnToHolding(true),
    units(units_),
    magnitude(magnitude_)
{
    connectAll();
}

WaveformAmplitudeParams::WaveformAmplitudeParams(const WaveformAmplitudeParams& other) :
    QObject(),
    startAmplitude(other.startAmplitude),
    stepSize(other.stepSize),
    endAmplitude(other.endAmplitude),
    multiStep(other.multiStep),
    returnToHolding(other.returnToHolding),
    units(other.units),
    magnitude(other.magnitude)
{
    connectAll();
}

void WaveformAmplitudeParams::connectAll() {
    connect(&startAmplitude, SIGNAL(valueChanged(double)), this, SLOT(adjustStepSize()));
    connect(&stepSize, SIGNAL(valueChanged(double)), this, SLOT(adjustStepSize()));
    connect(&endAmplitude, SIGNAL(valueChanged(double)), this, SLOT(adjustStepSize()));

    connect(&startAmplitude, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
    connect(&stepSize, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
    connect(&endAmplitude, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
    connect(&multiStep, SIGNAL(valueChanged(bool)), this, SLOT(notifyObserver()));
    connect(&returnToHolding, SIGNAL(valueChanged(bool)), this, SLOT(notifyObserver()));
}

void WaveformAmplitudeParams::adjustStepSize() {
    double delta = endAmplitude.value() - startAmplitude.value();
    double step = stepSize.value();

    if (step == 0) {
        stepSize.setValue(delta >= 0 ? stepSize.getStepSize() : -stepSize.getStepSize());
    }
    else {
        if ((delta > 0 && step < 0) || (delta < 0 && step > 0)) {
            stepSize.setValue(-stepSize.value());
        }
    }
}

void WaveformAmplitudeParams::notifyObserver() {
    emit valuesChanged();
}

string WaveformAmplitudeParams::toHtml() {
    ostringstream oss;
    oss << "<span style=\"color:blue\">" << QString::number(startAmplitude.value(), 'f', 1).toStdString().c_str() << units << "</span>";
    if (multiStep.value()) {
        oss << "..<span style=\"color:blue\">" << QString::number(endAmplitude.value(), 'f', 1).toStdString().c_str() << units << "</span>";
        oss << " by <span style=\"color:blue\">" << QString::number(stepSize.value(), 'f', 1).toStdString().c_str() << units << "</span>";
    }
    return oss.str();
}

void WaveformAmplitudeParams::set(const WaveformAmplitudeParams& other) {
    startAmplitude.set(other.startAmplitude);
    stepSize.set(other.stepSize);
    endAmplitude.set(other.endAmplitude);
    multiStep.set(other.multiStep);
    returnToHolding.set(other.returnToHolding);
    units = other.units;
    magnitude = other.magnitude;
}
//--------------------------------------------------------------------------
WaveformAmplitudeWidget::WaveformAmplitudeWidget(WaveformAmplitudeParams& params_, const char* type) :
    params(params_)
{
    startAmplitudeSpinBox = GUIUtil::createDoubleSpinBox(params.startAmplitude, params.units);
    stepSizeSpinBox = GUIUtil::createDoubleSpinBox(params.stepSize, params.units);
    endAmplitudeSpinBox = GUIUtil::createDoubleSpinBox(params.endAmplitude, params.units);

    multiStepCheckBox = new QCheckBox(tr("Multi-step sequence"));
    params.multiStep.connectTo(multiStepCheckBox);
    connect(&params.multiStep, SIGNAL(valueChanged(bool)), this, SLOT(setMultiStep(bool)));

    returnToHoldingCheckBox = new QCheckBox(tr((string("Return to holding ") + type + " after each step").c_str()));
    params.returnToHolding.connectTo(returnToHoldingCheckBox);
    returnToHoldingCheckBox->setChecked(params.returnToHolding.value());
    multiStepCheckBox->setChecked(params.multiStep.value());
    emit setMultiStep(params.multiStep.value());

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(GUIUtil::createLabeledWidget((string("Start ") + type + ":").c_str(), startAmplitudeSpinBox));
    layout->addWidget(multiStepCheckBox);
    layout->addLayout(GUIUtil::createLabeledWidget("Step size:", stepSizeSpinBox));
    layout->addLayout(GUIUtil::createLabeledWidget((string("End ") + type + ":").c_str(), endAmplitudeSpinBox));
    layout->addWidget(returnToHoldingCheckBox);
    layout->addStretch(1);

    setTitle(tr("Amplitude"));
    setLayout(layout);
}

void WaveformAmplitudeWidget::setMultiStep(bool checked)
{
    stepSizeSpinBox->setEnabled(checked);
    endAmplitudeSpinBox->setEnabled(checked);
    returnToHoldingCheckBox->setEnabled(checked);
}
