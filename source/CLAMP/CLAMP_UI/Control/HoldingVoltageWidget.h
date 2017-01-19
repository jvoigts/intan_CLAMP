#pragma once

#include <QGroupBox>

class QDoubleSpinBox;
class MySlider;

class HoldingVoltageWidget : public QGroupBox {
    Q_OBJECT

public:
    HoldingVoltageWidget();

    double valuemV();
    int valueInSteps(double offset);
	void setSliderEnabled(bool enabled);

private slots:
    void setHoldingVoltageInternal(double valueInmV);
    void setSliderValue(int);

private:
    MySlider* slider;
    QDoubleSpinBox*  spinBox;
};
