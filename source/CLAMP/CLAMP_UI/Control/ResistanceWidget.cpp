#include "ResistanceWidget.h"
#include <QtGui>
#include "common.h"
#include "SaveFile.h"
#include "GUIUtil.h"
#include "GlobalState.h"
#include "HoldingVoltageWidget.h"
#include <sstream>

using std::string;
using std::ostringstream;
using namespace CLAMP;

//--------------------------------------------------------------------------
ResistanceParams::ResistanceParams() :
    amplitudeInMv(10.0, 2.5, -255*2.5, +255*2.5),
    intervalS(1.0)
{
    timingParams.useFrequency.setValue(false);
    timingParams.frequency.setValue(10.0);
    timingParams.pulseTimeMs.setValue(50);
    timingParams.holdTimeMs.setValue(50);

    connect(&amplitudeInMv, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
    connect(&timingParams, SIGNAL(valuesChanged()), this, SLOT(notifyObserver()));
    connect(&intervalS, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
}

void ResistanceParams::set(const ResistanceParams& other) {
    amplitudeInMv.set(other.amplitudeInMv);
    timingParams.set(other.timingParams);
    intervalS.set(other.intervalS);
}

SimplifiedWaveform ResistanceParams::getSimplifiedWaveform(double samplingRate, int holdingValue, double pipetteOffset) {
    unsigned int numRepsHold, numRepsStep;
    timingParams.getNumReps(samplingRate, numRepsHold, numRepsStep);
    int stepValue = amplitudeInMv.valueAsSteps();
    int measureValue = holdingValue + stepValue;

    SimplifiedWaveform simplifiedWaveform;
    simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, numRepsHold, 0, false, false));
    simplifiedWaveform.push_back(WaveformSegment(0, measureValue, numRepsStep, 0, true, true));
    simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, 1, 0, false, false)); // Return to holding value at the very end

    simplifiedWaveform.setStepSize(2.5e-3, pipetteOffset);

    simplifiedWaveform.interval = intervalS.value();
    return simplifiedWaveform;
}

string ResistanceParams::toHtml() {
    ostringstream oss;
    oss << "<span style=\"color:blue\">" << QString::number(amplitudeInMv.value(), 'f', 1).toStdString().c_str() << " mV</span>";
    oss << timingParams.toHtml().c_str();
    oss << " every <span style=\"color:blue\">" << QString::number(intervalS.value(), 'f', 1).toStdString().c_str() << " s</span>";
    return oss.str();

}

void ResistanceParams::notifyObserver() {
    emit valuesChanged();
}

//--------------------------------------------------------------------------
ResistanceParamsDisplay::ResistanceParamsDisplay(ResistanceParams& params_) :
    params(params_) 
{
    QHBoxLayout* layout = new QHBoxLayout();

    label = new QLabel();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(label);
    setLayout(layout);

    connect(&params, SIGNAL(valuesChanged()), this, SLOT(redoLayout()));
    redoLayout();
}

void ResistanceParamsDisplay::redoLayout() {
    label->setText(params.toHtml().c_str());
}

void ResistanceParamsDisplay::mousePressEvent(QMouseEvent *) {
    ResistanceParams tmp;
    tmp.set(params);

    ResistanceDialog dialog(this, tmp);
    if (dialog.exec() == QDialog::Accepted) {
        params.set(tmp);
    }
}


//--------------------------------------------------------------------------
ResistanceDialog::ResistanceDialog(QWidget* parent, ResistanceParams& params_) :
    QDialog(parent),
    params(params_)
{

    QVBoxLayout* layout = new QVBoxLayout();
    QDoubleSpinBox* spinBox = GUIUtil::createDoubleSpinBox(params.amplitudeInMv, " mV");
    layout->addItem(GUIUtil::createLabeledWidget("Amplitude:", spinBox));

    layout->addWidget(new WaveformTimingWidget(params.timingParams));

    QDoubleSpinBox* readingEverySpinBox = GUIUtil::createDoubleSpinBox(255, 1, params.intervalS.value(), " s");
    readingEverySpinBox->setRange(0.1, 600);
    params.intervalS.connectTo(readingEverySpinBox);
    layout->addItem(GUIUtil::createLabeledWidget("Take a reading every:", readingEverySpinBox));


    layout->addStretch(1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);

    setLayout(layout);
}
