#pragma once

#include <QGroupBox>
#include "Thread.h"
#include "ClampController.h"

class GlobalState;
class Controller;
class FeedbackBandwidthWidget;
class QDoubleSpinBox;
class QPushButton;

class PipetteOffsetWidget : public QGroupBox {
    Q_OBJECT
public:
    PipetteOffsetWidget(GlobalState& state_, bool includeAutoButton, int unit_, Controller* controller_ = nullptr, FeedbackBandwidthWidget* feedback_ = nullptr);
    QPushButton* autoButton;

private slots:
    void autoCalibrate();
    void setAutoButtonEnable(bool value);
	void setPipetteOffsetHere(double value);
	void setValueHere(int unit_, double value);

private:
    GlobalState& state;
    Controller* controller;
    FeedbackBandwidthWidget* feedback;
	QDoubleSpinBox* pipetteOffsetSpinBox;
	int unit;
};

class PipetteOffsetThread : public Thread {
public:
    PipetteOffsetThread(GlobalState& state_, Controller& controller_, FeedbackBandwidthWidget& feedback_, int unit_);
    ~PipetteOffsetThread();

    void run() override;
    void done() override;

private:
    GlobalState& state;
    Controller& controller;
    FeedbackBandwidthWidget& feedback;
	int unit;

    void setRf(CLAMP::ClampConfig::ChipChannelList& channelList, double currentRange);
    double measureCurrent(CLAMP::ClampConfig::ChipChannelList& channelList, int voltageSetting);
};