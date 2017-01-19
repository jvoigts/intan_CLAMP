#pragma once

#include <QGroupBox>
#include "MVC.h"

class QRadioButton;
class QSpinBox;
class QDoubleSpinBox;

namespace CLAMP {
    class ClampController;
}

class TimingParams : public QObject {
    Q_OBJECT

public:
    TimingParams();

    BoolHolder useFrequency;
    DoubleHolder frequency;
    IntHolder holdTimeMs;
    IntHolder pulseTimeMs;

    void getNumReps(double samplingFreq, unsigned int& numRepsHold, unsigned int& numRepsStep);
    std::string toHtml();
    void set(const TimingParams& other);

signals:
    void valuesChanged();

private slots:
    void notifyObserver();
};


class WaveformTimingWidget : public QGroupBox {
    Q_OBJECT

public:
    WaveformTimingWidget(TimingParams& params_);

private slots :
    void setWaveformTimingType();

private:
    QRadioButton *useFreqRadioButton;
    QRadioButton *useDurationRadioButton;
    QDoubleSpinBox *frequencySpinBox;
    QSpinBox *pulseDurationSpinBox;
    QSpinBox *holdDurationSpinBox;

    TimingParams& params;
};
