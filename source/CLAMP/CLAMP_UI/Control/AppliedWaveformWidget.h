#pragma once

#include <QWidget>
#include <QDialog>
#include <QGroupBox>
#include "MVC.h"
#include "SimplifiedWaveform.h"
#include "WaveformTimingWidget.h"
#include "WaveformAmplitudeWidget.h"

class QWidget;
class QSpinBox;
class QLabel;
class TimingParams;
class WaveformAmplitudeParams;
class QDoubleSpinBox;
class GlobalState;

namespace CLAMP {
    class ClampController;
    class SimplifiedWaveform;
}

class NumCyclesWidget : public QWidget {
    Q_OBJECT

public:
    NumCyclesWidget(IntHolder& params_, const char* text=nullptr);

private:
    QSpinBox *numCyclesSpinBox;
    IntHolder& params;
};

class WaveformCreator {
public:
    static CLAMP::SimplifiedWaveform createSimplifiedWaveform(double samplingRate, IntHolder& cyclesParams, WaveformAmplitudeParams& amplitude, TimingParams& timing, int holdingValue, double appliedStepSize, double offset);
    static CLAMP::SimplifiedWaveform createMultiStep(bool returnToHolding, int holdingValue, unsigned int numRepsHold, unsigned int numRepsStep, unsigned int numCycles, int startValue, int stepSize, int endValue, double appliedStepSize, double offset);
    static CLAMP::SimplifiedWaveform createPulses(unsigned int numCycles, int holdingValue, unsigned int numRepsHold, int pulseValue, unsigned int numRepsStep, double appliedStepSize, double offset);
};

class MultistepParams : public QObject {
    Q_OBJECT

public:
    IntHolder numCycles;
    TimingParams timingParams;
    WaveformAmplitudeParams amplitudeParams;

    MultistepParams(const WaveformAmplitudeParams& amplitude);
    void set(const MultistepParams& other);
    CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, int holdingValue, double offset);
    std::string toHtml();

signals:
    void valuesChanged();

private slots:
    void notifyObserver();
};

class MultistepParamsDisplay : public QWidget {
    Q_OBJECT

public:
    MultistepParamsDisplay(MultistepParams& params_, const char* type_);

public slots:
    void redoLayout();

protected:
    void mousePressEvent(QMouseEvent *ev) override;

private:
    MultistepParams& params;
    const char* type;

    QLabel* label;
};

class ArbWaveformParamsDisplay : public QWidget {
	Q_OBJECT

public:
	ArbWaveformParamsDisplay(bool iClamp_);
	CLAMP::SimplifiedWaveform getSimplifiedWaveform(int holdingValue, double pipetteOffset);

signals:
	void arbWaveformLoaded();

private:
	QPushButton* loadArbButton;
	QLabel* label;
	CLAMP::SimplifiedWaveform simplifiedWaveform;
	bool iClamp;

private slots:
	void loadArbWaveform();
};

class MultistepDialog : public QDialog {
    Q_OBJECT

public:
    MultistepDialog(QWidget* parent, MultistepParams& params_, const char* type);

private:
    MultistepParams& params;
};

class PulseTrainParams : public QObject {
    Q_OBJECT

public:
    IntHolder numPulses;
    StepDoubleHolder amplitude;
    TimingParams timingParams;
    const char* units;

    PulseTrainParams(double initialValue, double step, int maxValue, double magnitude_, const char* units_);

    void set(const PulseTrainParams& other);
    CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, int holdingValue, double offset);
    std::string toHtml();

signals:
    void valuesChanged();

private slots:
    void notifyObserver();

private:
    double magnitude;
};

class PulseTrainDialog : public QDialog {
    Q_OBJECT

public:
    PulseTrainDialog(QWidget* parent, PulseTrainParams& params_);

private:
    PulseTrainParams& params;
};

class PulseTrainParamsDisplay : public QWidget {
    Q_OBJECT

public:
    PulseTrainParamsDisplay(PulseTrainParams& params_);

public slots:
    void redoLayout();

protected:
    void mousePressEvent(QMouseEvent *ev) override;

private:
    PulseTrainParams& params;

    QLabel* label;
};

