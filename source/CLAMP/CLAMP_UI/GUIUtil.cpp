#include "GUIUtil.h"
#include <QtGui>
#include "common.h"
#include "MVC.h"

QHBoxLayout* GUIUtil::createLabeledWidget(const char* text, QWidget* widget) {
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(new QLabel(text));
    layout->addStretch(1);
    layout->addWidget(widget);

    return layout;
}


QSpinBox* GUIUtil::createDurationSpinBox(int defaultValue) {
    QSpinBox* durationSpinBox = new QSpinBox();
    durationSpinBox->setRange(1, 10 * 1000); // 10 seconds
    durationSpinBox->setSingleStep(1);
    durationSpinBox->setSuffix(" ms");
    durationSpinBox->setValue(defaultValue);
    durationSpinBox->setKeyboardTracking(false);
    return durationSpinBox;
}

QDoubleSpinBox* GUIUtil::createDoubleSpinBox(StepDoubleHolder& param, const char* units) {
    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setRange(param.getMin(), param.getMax());
    spinBox->setSingleStep(param.getStepSize());
    spinBox->setSuffix(units);
    spinBox->setValue(param.value());
    spinBox->setDecimals(1);
    spinBox->setKeyboardTracking(false);

    param.connectTo(spinBox);
    return spinBox;
}

QDoubleSpinBox* GUIUtil::createDoubleSpinBox(int maxValue, double stepSize, double defaultValue, const char* units) {
    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setRange(-maxValue * stepSize, maxValue * stepSize);
    spinBox->setSingleStep(stepSize);
    spinBox->setSuffix(units);
    spinBox->setValue(defaultValue);
    spinBox->setDecimals(1);
    spinBox->setKeyboardTracking(false);
    return spinBox;
}

QDoubleSpinBox* GUIUtil::createDoubleSpinBoxZeroFloor(int maxValue, double stepSize, double defaultValue, const QString& units) {
	QDoubleSpinBox* spinBox = new QDoubleSpinBox();
	spinBox->setRange(0, maxValue);
	spinBox->setSingleStep(stepSize);
	spinBox->setSuffix(units);
	spinBox->setValue(defaultValue);
	spinBox->setDecimals(1);
	spinBox->setKeyboardTracking(false);
	return spinBox;
}

QCheckBox* GUIUtil::createLockBox(BoolHolder& param) {
    QCheckBox* cb = new QCheckBox();
    cb->setStyleSheet(
        "QCheckBox::indicator{                                  \
            width: 18px;                                        \
            height: 18px;                                       \
        }                                                       \
                                                                \
        QCheckBox::indicator:checked                            \
        {                                                       \
            image: url(:/images/Unlocked.png);                  \
        }                                                       \
        QCheckBox::indicator:unchecked                          \
        {                                                       \
            image: url(:/images/Locked.png);                    \
        }"
    );
    cb->setChecked(param.value());
    param.connectTo(cb);
    return cb;
}

int GUIUtil::toBoardSteps(double value, double stepSize) {
    int ret = lround(value/ stepSize);
    return ret;
}

int GUIUtil::toBoardStepsWithValueOffset(QDoubleSpinBox& spinbox, double offset) {
    int ret = toBoardSteps(spinbox.value() + offset, spinbox.singleStep());

    int maxMagnitude = spinbox.maximum() / spinbox.singleStep();
    ret = std::min(ret, maxMagnitude);
    ret = std::max(ret, -maxMagnitude);
    return ret;
}

void GUIUtil::drawText(QPainter & painter, int x, int y, Qt::Alignment flags, const QString & text)
{
    int w = painter.fontMetrics().width(text);
    int h = painter.fontMetrics().height();

    if (flags & Qt::AlignHCenter) {
        x -= w / 2;
    }
    else if (flags & Qt::AlignRight) {
        x -= w;
    }

    if (flags & Qt::AlignVCenter) {
        y -= h / 2;
    }
    else if (flags & Qt::AlignBottom) {
        y -= h;
    }

    painter.drawText(x, y, w, h, flags, text);
}

