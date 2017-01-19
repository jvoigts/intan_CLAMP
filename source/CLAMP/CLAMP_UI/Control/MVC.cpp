#include "MVC.h"
#include "common.h"
#include <algorithm>

//--------------------------------------------------------------------------
void IntHolder::connectTo(QObject* other) {
    connect(this, SIGNAL(valueChanged(int)), other, SLOT(setValue(int)));
    connect(other, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
}

//--------------------------------------------------------------------------
void DoubleHolder::connectTo(QObject* other) {
    connect(this, SIGNAL(valueChanged(double)), other, SLOT(setValue(double)));
    connect(other, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
}

//--------------------------------------------------------------------------
int StepDoubleHolder::valueAsSteps(int offset) const {
    int steps = lround(value() / m_stepSize);
    if (offset != 0) {
        steps = steps + offset;

        steps = std::min(steps, static_cast<int>(lround(getMax() / m_stepSize)));
        steps = std::max(steps, static_cast<int>(lround(getMin() / m_stepSize)));
    }
    return steps;

}
//--------------------------------------------------------------------------
void BoolHolder::connectTo(QObject* other) {
    connect(this, SIGNAL(valueChanged(bool)), other, SLOT(setChecked(bool)));
    connect(other, SIGNAL(toggled(bool)), this, SLOT(setValue(bool)));
}
