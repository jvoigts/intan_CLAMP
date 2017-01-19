#pragma once

#include <QGroupBox>
#include "MVC.h"

class QDoubleSpinBox;
class QCheckBox;
class QLayout;

class WaveformAmplitudeParams : public QObject {
    Q_OBJECT

public:
    StepDoubleHolder startAmplitude;
    StepDoubleHolder stepSize;
    StepDoubleHolder endAmplitude;
    BoolHolder multiStep;
    BoolHolder returnToHolding;
    const char* units;
    double magnitude;

    WaveformAmplitudeParams(double stepSize_, int maxValue_, double defaultStart, double defaultStep, double defaultEnd, const char* units_, double magnitude);
    WaveformAmplitudeParams(const WaveformAmplitudeParams& other);

    std::string toHtml();
    void set(const WaveformAmplitudeParams& other);

signals:
    void valuesChanged();

private slots:
    void notifyObserver();
    void adjustStepSize();

private:
    void connectAll();
};

class WaveformAmplitudeWidget : public QGroupBox {
    Q_OBJECT

public:
    WaveformAmplitudeParams& params;

    WaveformAmplitudeWidget(WaveformAmplitudeParams& params_, const char* type);

private slots :
    void setMultiStep(bool checked);

private:
    QDoubleSpinBox *startAmplitudeSpinBox;
    QDoubleSpinBox *stepSizeSpinBox;
    QDoubleSpinBox *endAmplitudeSpinBox;

    QCheckBox *multiStepCheckBox;
    QCheckBox *returnToHoldingCheckBox;
};
