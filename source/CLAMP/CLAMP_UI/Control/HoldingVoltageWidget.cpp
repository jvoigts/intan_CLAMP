#include "HoldingVoltageWidget.h"
#include <QtGui>
#include "common.h"
#include "GUIUtil.h"

const double stepSize = 2.5;  // 2.5 mV

//--------------------------------------------------------------------------
class MySlider : public QSlider
{
protected:
    void mousePressEvent(QMouseEvent * event)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (orientation() == Qt::Vertical)
                setValue(minimum() + ((maximum() - minimum()) * (height() - event->y())) / height());
            else
                setValue(minimum() + ((maximum() - minimum()) * event->x()) / width());

            event->accept();
        }
        QSlider::mousePressEvent(event);
    }
};

//--------------------------------------------------------------------------
HoldingVoltageWidget::HoldingVoltageWidget() {
    slider = new MySlider();
    slider->setOrientation(Qt::Orientation::Horizontal);
    slider->setRange(-40, 40);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(10);
    slider->setValue(0);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setSliderValue(int)));

    spinBox = GUIUtil::createDoubleSpinBox(255, stepSize, 0, " mV");
    spinBox->setStyleSheet(QString::fromUtf8("color:blue; background:transparent; border:none;"));
    spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(setHoldingVoltageInternal(double)));

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(spinBox, 0, 0, 2, 1);
    grid->addWidget(slider, 0, 1, 1, 9);

    QLabel* tmp = new QLabel("-100");
    tmp->setAlignment(Qt::AlignCenter);
    grid->addWidget(tmp, 1, 1, 1, 1);

    tmp = new QLabel("0");
    tmp->setAlignment(Qt::AlignCenter);
    grid->addWidget(tmp, 1, 5, 1, 1);

    tmp = new QLabel("100");
    tmp->setAlignment(Qt::AlignCenter);
    grid->addWidget(tmp, 1, 9, 1, 1);

    setTitle(tr("Holding Voltage"));
    setLayout(grid);
}

void HoldingVoltageWidget::setSliderValue(int) {
    setHoldingVoltageInternal(slider->value() * stepSize);
}

void HoldingVoltageWidget::setHoldingVoltageInternal(double valueInmV) {
    valueInmV = std::max(-100.0, valueInmV);
    valueInmV = std::min(100.0, valueInmV);
    spinBox->setValue(valueInmV);
    slider->setValue(round(valueInmV / stepSize));
}

double HoldingVoltageWidget::valuemV() {
    return spinBox->value();
}

int HoldingVoltageWidget::valueInSteps(double offset) {
    return GUIUtil::toBoardStepsWithValueOffset(*spinBox, offset);
}

void HoldingVoltageWidget::setSliderEnabled(bool enabled) {
	slider->setEnabled(enabled);
}
