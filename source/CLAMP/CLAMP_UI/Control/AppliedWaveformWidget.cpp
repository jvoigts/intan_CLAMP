#include "AppliedWaveformWidget.h"
#include "WaveformTimingWidget.h"
#include "WaveformAmplitudeWidget.h"
#include "SaveFile.h"
#include <QtGui>
#include "common.h"
#include "GUIUtil.h"
#include "globalconstants.h"
#include "GlobalState.h"
#include <sstream>

using namespace CLAMP;
using std::ostringstream;
using std::string;

//--------------------------------------------------------------------------
NumCyclesWidget::NumCyclesWidget(IntHolder& params_, const char* text) :
    params(params_)
{
    numCyclesSpinBox = new QSpinBox();
    numCyclesSpinBox->setRange(1, 99);
    numCyclesSpinBox->setSingleStep(1);
    numCyclesSpinBox->setValue(params.value());
    numCyclesSpinBox->setKeyboardTracking(false);

    const char* text2 = (text == nullptr) ? "Number of cycles:" : text;
    QHBoxLayout *numCyclesLayout = GUIUtil::createLabeledWidget(text2, numCyclesSpinBox);
    numCyclesLayout->addStretch(10);

    setLayout(numCyclesLayout);

    params.connectTo(numCyclesSpinBox);
}

//--------------------------------------------------------------------------
SimplifiedWaveform WaveformCreator::createSimplifiedWaveform(double samplingRate, IntHolder& cyclesParams, WaveformAmplitudeParams& amplitude, TimingParams& timing, int holdingValue, double appliedStepSize, double offset) {
    unsigned int numCycles = cyclesParams.value();

    int startValue = amplitude.startAmplitude.valueAsSteps(holdingValue);
    int stepSize = amplitude.stepSize.valueAsSteps(); // Offset doesn't apply here
    int endValue = amplitude.endAmplitude.valueAsSteps(holdingValue);
    bool multistep = amplitude.multiStep.value();
    bool returnToHolding = amplitude.returnToHolding.value();

    unsigned int numRepsHold, numRepsStep;
    timing.getNumReps(samplingRate, numRepsHold, numRepsStep);

    int delta = endValue - startValue;
    if ((delta < 0 && stepSize > 0) || (delta > 0 && stepSize < 0)) {
        multistep = false;
    }

    if (multistep) {
        return createMultiStep(returnToHolding, holdingValue, numRepsHold, numRepsStep, numCycles, startValue, stepSize, endValue, appliedStepSize, offset);
    }
    else {
        return createPulses(numCycles, holdingValue, numRepsHold, startValue, numRepsStep, appliedStepSize, offset);
    }
}

SimplifiedWaveform WaveformCreator::createMultiStep(bool returnToHolding, int holdingValue, unsigned int numRepsHold, unsigned int numRepsStep, unsigned int numCycles, int startValue, int stepSize, int endValue, double appliedStepSize, double offset) {
    SimplifiedWaveform simplifiedWaveform;
    if (returnToHolding) {
        unsigned int offset = 0;
        simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, numRepsHold / 2, offset, false, false));
        for (unsigned int cycle = 0; cycle < numCycles; cycle++) {
            for (int value = startValue, i = 1; value <= endValue; value += stepSize, i++) {
                simplifiedWaveform.push_back(WaveformSegment(i, holdingValue, numRepsHold - numRepsHold / 2, offset, false, false));
                simplifiedWaveform.push_back(WaveformSegment(i, value, numRepsStep, offset, true, true));
                simplifiedWaveform.push_back(WaveformSegment(i, holdingValue, numRepsHold / 2, offset, false, false));
                offset += numRepsHold + numRepsStep;
            }
            offset -= numRepsHold + numRepsStep;
        }
        simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, numRepsHold - numRepsHold / 2, offset, false, false));
    }
    else {
        unsigned int offset = 0;
        simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, numRepsHold, offset, false, false));
        for (unsigned int cycle = 0; cycle < numCycles; cycle++) {
            for (int value = startValue, i = 1; value <= endValue; value += stepSize, i++) {
                simplifiedWaveform.push_back(WaveformSegment(i, value, numRepsStep, offset, true, true));
                offset += numRepsStep;
            }
            offset -= numRepsStep;
            simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, numRepsHold, offset, false, false));
        }
    }
    simplifiedWaveform.setStepSize(appliedStepSize, offset);
    return simplifiedWaveform;
}

SimplifiedWaveform WaveformCreator::createPulses(unsigned int numCycles, int holdingValue, unsigned int numRepsHold, int pulseValue, unsigned int numRepsStep, double appliedStepSize, double offset) {
    SimplifiedWaveform simplifiedWaveform;
    for (unsigned int cycle = 0; cycle < numCycles; cycle++) {
        simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, numRepsHold, 0, false, false));
        simplifiedWaveform.push_back(WaveformSegment(0, pulseValue, numRepsStep, 0, true, true));
    }
    simplifiedWaveform.push_back(WaveformSegment(0, holdingValue, numRepsHold, 0, false, false));
    simplifiedWaveform.setStepSize(appliedStepSize, offset);
    return simplifiedWaveform;
}

//--------------------------------------------------------------------------
MultistepParams::MultistepParams(const WaveformAmplitudeParams& amplitude) :
    numCycles(1),
    amplitudeParams(amplitude)
{
    connect(&numCycles, SIGNAL(valueChanged(int)), this, SLOT(notifyObserver()));
    connect(&amplitudeParams, SIGNAL(valuesChanged()), this, SLOT(notifyObserver()));
    connect(&timingParams, SIGNAL(valuesChanged()), this, SLOT(notifyObserver()));
}

SimplifiedWaveform MultistepParams::getSimplifiedWaveform(double samplingRate, int holdingValue, double offset) {
    return WaveformCreator::createSimplifiedWaveform(samplingRate, numCycles, amplitudeParams, timingParams, holdingValue, amplitudeParams.startAmplitude.getStepSize() * amplitudeParams.magnitude, offset);
}

string MultistepParams::toHtml() {
    ostringstream oss;
    oss << amplitudeParams.toHtml().c_str();
//    if (amplitudeParams.multiStep.value()) {
//        oss << "<br>";
//    }

    oss << timingParams.toHtml().c_str();
    if (numCycles.value() > 1) {
        oss << "; <span style=\"color:blue\">" << numCycles.value() << "</span> cycles ";
    }
    return oss.str();
}

void MultistepParams::notifyObserver() {
    emit valuesChanged();
}

void MultistepParams::set(const MultistepParams& other) {
    numCycles.set(other.numCycles);
    amplitudeParams.set(other.amplitudeParams);
    timingParams.set(other.timingParams);
}

//--------------------------------------------------------------------------
MultistepDialog::MultistepDialog(QWidget* parent, MultistepParams& params_, const char* type) :
    QDialog(parent),
    params(params_)
{
    QVBoxLayout* layout = new QVBoxLayout();

    layout->addWidget(new NumCyclesWidget(params.numCycles));
    layout->addWidget(new WaveformTimingWidget(params.timingParams));
    layout->addWidget(new WaveformAmplitudeWidget(params.amplitudeParams, type));


    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);

    setLayout(layout);
}

//--------------------------------------------------------------------------
MultistepParamsDisplay::MultistepParamsDisplay(MultistepParams& params_, const char* type_) :
    params(params_),
    type(type_)
{
    QHBoxLayout* layout = new QHBoxLayout();

    label = new QLabel();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(label);
    setLayout(layout);

    connect(&params, SIGNAL(valuesChanged()), this, SLOT(redoLayout()));
    redoLayout();
}

void MultistepParamsDisplay::redoLayout() {
    label->setText(params.toHtml().c_str());
}

void MultistepParamsDisplay::mousePressEvent(QMouseEvent *) {
    MultistepParams tmp(params.amplitudeParams);
    tmp.set(params);

    MultistepDialog dialog(this, tmp, type);
    if (dialog.exec() == QDialog::Accepted) {
        params.set(tmp);
    }
}

//--------------------------------------------------------------------------
ArbWaveformParamsDisplay::ArbWaveformParamsDisplay(bool iClamp_) :
	iClamp(iClamp_)
{
	QHBoxLayout *layout = new QHBoxLayout();

	loadArbButton = new QPushButton("Load", this);
	label = new QLabel("<span style=\"color:blue\">no waveform loaded</span>");

	connect(loadArbButton, SIGNAL(clicked()), this, SLOT(loadArbWaveform()));

	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(label);
	layout->addStretch(1);
	layout->addWidget(loadArbButton);
	setLayout(layout);
}

void ArbWaveformParamsDisplay::loadArbWaveform()
{
    /*

	QString filename;

	filename = QFileDialog::getOpenFileName(this,
		tr("Select Arbitrary Waveform File"), ".",
		tr("Arbitrary Waveform Files (*.arb)"));

	QFile file(filename);

	if (file.open(QIODevice::ReadOnly)) {
		QFileInfo newFileInfo(filename);
		int lineNum = 0;

		QString header = file.readLine();
		lineNum++;
		if (header.startsWith("ARBV")) {
			if (iClamp) {
				QMessageBox::warning(this, tr("CLAMP Controller"), tr("Incorrect header.  Current clamp arbitrary waveform files must begin with ARBI."), QMessageBox::Ok);
				file.close();
				return;
			}
		}
		else if (header.startsWith("ARBI")) {
			if (!iClamp) {
				QMessageBox::warning(this, tr("CLAMP Controller"), tr("Incorrect header.  Voltage clamp arbitrary waveform files must begin with ARBV."), QMessageBox::Ok);
				file.close();
				return;
			}
		}
		else {
			QMessageBox::warning(this, tr("CLAMP Controller"), tr("Incorrect header.  First line of arbitrary waveform must be ARBV or ARBI."), QMessageBox::Ok);
			file.close();
			return;
		}

		simplifiedWaveform.erase();
		int waveformNumber;
		int appliedDiscreteValue;
		int numTimesteps;
		int tOffset;
		bool markerOut;
		bool digOut;
		QString str;

		while (!file.atEnd()) {

			QByteArray line = file.readLine();
			lineNum++;
			QStringList stringList;
			for (auto& index : line.split(',')) {
				stringList.push_back(index);
			}

			if (stringList.size() == 6) {
				waveformNumber = stringList[0].toInt();  // TODO: add error checking for this parameter
				appliedDiscreteValue = stringList[1].toInt();
				numTimesteps = stringList[2].toInt();
				tOffset = stringList[3].toInt();  // TODO: add error checking for this parameter
				markerOut = (stringList[4].toInt() != 0);
				digOut = (stringList[5].toInt() != 0);
			}
			else if (stringList.size() == 4) {
				waveformNumber = 0;
				appliedDiscreteValue = stringList[0].toInt();
				numTimesteps = stringList[1].toInt();
				tOffset = 0;
				markerOut = (stringList[2].toInt() != 0);
				digOut = (stringList[3].toInt() != 0);
			}
			else if (stringList.size() == 2) {
				waveformNumber = 0;
				appliedDiscreteValue = stringList[0].toInt();
				numTimesteps = stringList[1].toInt();
				tOffset = 0;
				markerOut = appliedDiscreteValue != 0;
				digOut = appliedDiscreteValue != 0;
			}
			else if (stringList.size() > 1) {
				DEBUGOUT(stringList.size() << endl);
				QMessageBox::warning(this, tr("CLAMP Controller"), tr("Syntax error in arbitrary waveform file.  Incorrect number of arguments in line ") + str.setNum(lineNum) + ".", QMessageBox::Ok);
				file.close();
				return;
			}

			if (stringList.size() > 1) {
				if (numTimesteps < 1) {
					QMessageBox::warning(this, tr("CLAMP Controller"), tr("Syntax error in arbitrary waveform file, line ") + str.setNum(lineNum) + tr(".  Duration must be greater than zero."), QMessageBox::Ok);
					file.close();
					return;
				}
				else if (numTimesteps > 65535) {
					QMessageBox::warning(this, tr("CLAMP Controller"), tr("Syntax error in arbitrary waveform file, line ") + str.setNum(lineNum) + tr(".  Duration must be less than 65536."), QMessageBox::Ok);
					file.close();
					return;
				}
				if (iClamp) {
					if (appliedDiscreteValue < -127 || appliedDiscreteValue > 127) {
						QMessageBox::warning(this, tr("CLAMP Controller"), tr("Syntax error in arbitrary waveform file, line ") + str.setNum(lineNum) + tr(".  Current clamp level must fall in the range of -127 to +127."), QMessageBox::Ok);
						file.close();
						return;
					}
				}
				else {
					if (appliedDiscreteValue < -255 || appliedDiscreteValue > 255) {
						QMessageBox::warning(this, tr("CLAMP Controller"), tr("Syntax error in arbitrary waveform file, line ") + str.setNum(lineNum) + tr(".  Voltage clamp level must fall in the range of -255 to +255."), QMessageBox::Ok);
						file.close();
						return;
					}

				}
				simplifiedWaveform.push_back(WaveformSegment(waveformNumber, appliedDiscreteValue, numTimesteps, tOffset, markerOut, digOut));
			}
		}


		if (simplifiedWaveform.size() > 0) {
			QString displayedName = newFileInfo.baseName();
			const int MAX_NUM_CHARACTERS_DISPLAYED = 25;
			if (displayedName.length() > MAX_NUM_CHARACTERS_DISPLAYED) {
				displayedName.truncate(MAX_NUM_CHARACTERS_DISPLAYED);
				displayedName += QString("...");
			}
			label->setText(QString("<span style=\"color:blue\">") + QString(displayedName) + QString("</span>"));
			loadArbButton->setText("Reload");
			emit arbWaveformLoaded();
		}
		else {
			QMessageBox::warning(this, tr("CLAMP Controller"), tr("Syntax error in arbitrary waveform file."), QMessageBox::Ok);
		}

		file.close();
    } // if (file.open(QIODevice::ReadOnly)) {
    */
}

SimplifiedWaveform ArbWaveformParamsDisplay::getSimplifiedWaveform(int holdingValue, double pipetteOffset)
{
    SimplifiedWaveform adjustedWaveform;
    adjustedWaveform = simplifiedWaveform;
    for (unsigned int i = 0; i < adjustedWaveform.waveform.size(); i++) {
        adjustedWaveform.waveform[i].appliedDiscreteValue += holdingValue;
    }
    return adjustedWaveform;
}

//--------------------------------------------------------------------------
PulseTrainParams::PulseTrainParams(double initialValue, double step, int maxValue, double magnitude_, const char* units_) :
    numPulses(1),
    amplitude(initialValue, step, -maxValue * step, maxValue * step),
    units(units_),
    magnitude(magnitude_)
{
    connect(&numPulses, SIGNAL(valueChanged(int)), this, SLOT(notifyObserver()));
    connect(&amplitude, SIGNAL(valueChanged(double)), this, SLOT(notifyObserver()));
    connect(&timingParams, SIGNAL(valuesChanged()), this, SLOT(notifyObserver()));
}

SimplifiedWaveform PulseTrainParams::getSimplifiedWaveform(double samplingRate, int holdingValue, double offset) {
    unsigned int numRepsHold, numRepsStep;
    timingParams.getNumReps(samplingRate, numRepsHold, numRepsStep);
    int value = amplitude.valueAsSteps(holdingValue);
    return WaveformCreator::createPulses(numPulses.value(), holdingValue, numRepsHold, value, numRepsStep, amplitude.getStepSize() * magnitude, offset);
}

string PulseTrainParams::toHtml() {
    ostringstream oss;
    if (numPulses.value() > 1) {
        oss << "<span style=\"color:blue\">" << numPulses.value() << "</span> pulses of ";
    }
    oss << "<span style=\"color:blue\">" << QString::number(amplitude.value(), 'f', 1).toStdString().c_str() << units << "</span>";
    oss << timingParams.toHtml().c_str();
    return oss.str();
}

void PulseTrainParams::notifyObserver() {
    emit valuesChanged();
}

void PulseTrainParams::set(const PulseTrainParams& other) {
    numPulses.set(other.numPulses);
    amplitude.set(other.amplitude);
    timingParams.set(other.timingParams);
    magnitude = other.magnitude;
    units = other.units;
}

//--------------------------------------------------------------------------
PulseTrainParamsDisplay::PulseTrainParamsDisplay(PulseTrainParams& params_) : 
    params(params_) 
{
    QHBoxLayout* layout = new QHBoxLayout();

    label = new QLabel();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    setLayout(layout);

    connect(&params, SIGNAL(valuesChanged()), this, SLOT(redoLayout()));
    redoLayout();
}

void PulseTrainParamsDisplay::redoLayout() {
    label->setText(params.toHtml().c_str());
}

void PulseTrainParamsDisplay::mousePressEvent(QMouseEvent *) {
    PulseTrainParams tmp(0.0, 0.0, 0, 0, "");
    tmp.set(params);

    PulseTrainDialog dialog(this, tmp);
    if (dialog.exec() == QDialog::Accepted) {
        params.set(tmp);
    }
}

//--------------------------------------------------------------------------
PulseTrainDialog::PulseTrainDialog(QWidget* parent, PulseTrainParams& params_) :
    QDialog(parent),
    params(params_)
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(new NumCyclesWidget(params.numPulses, "Number of pulses:"));

    QDoubleSpinBox* spinBox = GUIUtil::createDoubleSpinBox(params.amplitude, params.units);
    layout->addItem(GUIUtil::createLabeledWidget("Amplitude:", spinBox));

    layout->addWidget(new WaveformTimingWidget(params.timingParams));

    layout->addStretch(1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);

    setLayout(layout);
}

