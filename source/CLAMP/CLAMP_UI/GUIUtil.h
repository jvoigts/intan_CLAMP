#pragma once

#include <QtGui>
class QHBoxLayout;
class QWidget;
class QSpinBox;
class QDoubleSpinBox;
class QPainter;
class QCheckBox;
class StepDoubleHolder;
class BoolHolder;

class GUIUtil {
public:
    static QHBoxLayout* createLabeledWidget(const char* text, QWidget* widget);
    static QSpinBox* createDurationSpinBox(int defaultValue);
    static QDoubleSpinBox* createDoubleSpinBox(StepDoubleHolder& param, const char* units);
    static QDoubleSpinBox* createDoubleSpinBox(int maxValue, double stepSize, double defaultValue, const char* units);
	static QDoubleSpinBox* createDoubleSpinBoxZeroFloor(int maxValue, double stepSize, double defaultValue, const QString& units);
    static QCheckBox* createLockBox(BoolHolder& param);
    static int toBoardStepsWithValueOffset(QDoubleSpinBox& spinbox, double offset);
    static void drawText(QPainter & painter, int x, int y, Qt::Alignment flags, const QString & text);

private:
    static int toBoardSteps(double value, double stepSize);
};

