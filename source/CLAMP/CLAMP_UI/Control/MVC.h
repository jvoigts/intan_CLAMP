#pragma once

#include <QObject>

class IntHolder : public QObject {
    Q_OBJECT

public:
    IntHolder(int initialValue) : QObject(), m_value(initialValue) {}
    IntHolder(const IntHolder& other) : QObject(), m_value(other.m_value) {}
    int value() const { return m_value; }
    void connectTo(QObject* other);
    void set(const IntHolder& other) { setValue(other.value()); }

signals:
    void valueChanged(int value);

public slots:
    void setValue(int value) {
        m_value = value;
        emit valueChanged(value);
    }

private:
    int m_value;
};

class DoubleHolder : public QObject {
    Q_OBJECT

public:
    DoubleHolder(double initialValue) : QObject(), m_value(initialValue) {}
    DoubleHolder(const DoubleHolder& other) : QObject(), m_value(other.m_value) {}
    double value() const { return m_value; }
    void connectTo(QObject* other);
    void set(const DoubleHolder& other) { setValue(other.value()); }

signals:
    void valueChanged(double value);

public slots:
    void setValue(double value) {
        m_value = value;
        emit valueChanged(value);
    }

private:
    double m_value;
};

class StepDoubleHolder : public DoubleHolder {
public:
    StepDoubleHolder(double initialValue, double stepSize, double min_, double max_) : DoubleHolder(initialValue), m_stepSize(stepSize), m_min(min_), m_max(max_) {}
    StepDoubleHolder(const StepDoubleHolder& other) : DoubleHolder(other), m_stepSize(other.m_stepSize), m_min(other.m_min), m_max(other.m_max) {}
    double getStepSize() const { return m_stepSize; }
    int valueAsSteps(int offset = 0) const;
    void set(const StepDoubleHolder& other) { 
        m_stepSize = other.m_stepSize;
        m_min = other.m_min;
        m_max = other.m_max;
        setValue(other.value());
    }
    double getMin() const { return m_min; }
    double getMax() const { return m_max; }

private:
    double m_stepSize;
    double m_min;
    double m_max;
};

class BoolHolder : public QObject {
    Q_OBJECT

public:
	BoolHolder() : QObject(), m_value(false) {}
	BoolHolder(bool initialValue) : QObject(), m_value(initialValue) {}
    BoolHolder(const BoolHolder& other) : QObject(), m_value(other.m_value) {}
    bool value() const { return m_value; }
    void connectTo(QObject* other);
    void set(const BoolHolder& other) { setValue(other.value()); }

signals:
    void valueChanged(bool value);

public slots:
    void setValue(bool value) {
        m_value = value;
        emit valueChanged(value);
    }

private:
    bool m_value;
};
