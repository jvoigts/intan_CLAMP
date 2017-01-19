#pragma once

#include <QGroupBox>
#include "Registers.h"

class QComboBox;
class QLabel;
class QString;
class QDoubleSpinBox;

namespace CLAMP {
    class Channel;
}

class FeedbackBandwidthWidget : public QGroupBox {
    Q_OBJECT

public:
    FeedbackBandwidthWidget(CLAMP::Channel& channel_);
    double getDesiredBandwidth() const;
    CLAMP::Registers::Register3::Resistance getResistanceEnum();
    bool valuesChanged();
	void setControlsEnabled(bool enabled);

signals:
	void feedbackResistanceChanged(int index);

private slots:
    void setResistor(int value);
    void setDesiredBandwidth();

private:
    QComboBox* currentScale;
    //QLabel* measuredResistance;
    //QLabel* minBandwidth;
    //QLabel* maxBandwidth;
    //QDoubleSpinBox* desiredBandwidth;
    //QLabel* actualBandwidth;
    //QLabel* feedbackCapacitor;

    CLAMP::Channel& channel;
    QString getBandwidthString(double value);
    void bandwidthChanged();

    void setResistorInternal(CLAMP::Registers::Register3::Resistance value);
    double getResistance();
    double getCapacitance();
    double getActualBandwidth();
    void setBestCapacitor();

    CLAMP::Registers::Register3 r3;
    CLAMP::Registers::Register4 r4;
};
