#pragma once

#include <QGroupBox>

class QDoubleSpinBox;
class MySlider;
class QPushButton;

class HoldingVoltageWidget : public QGroupBox {
    Q_OBJECT

public:
    HoldingVoltageWidget();
    QPushButton* minus70Button;
    QPushButton* zeroButton;


    double valuemV();
    int valueInSteps(double offset);
	void setSliderEnabled(bool enabled);

private slots:
    void setHoldingVoltageInternal(double valueInmV);
    void setSliderValue(int);
    void setminus70Button(bool value);
    void set0Button(bool value);

private:
    MySlider* slider;
    QDoubleSpinBox*  spinBox;
};
