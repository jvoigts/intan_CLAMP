#pragma once

#include "Thread.h"
#include <memory>
#include <vector>
#include "ClampController.h"
#include "Constants.h"
#include "CurrentClampWidget.h"
#include "ClampThread.h"
#include "Registers.h"
#include "SimplifiedWaveform.h"
#include "VoltageClampWidget.h"

class DisplayWindow;
class ControlWindow;
class GlobalState;
class Controller;
class VoltageClampWidget;
class CurrentClampWidget;
class CapacitiveCompensationController;
namespace CLAMP {
    class Board;
}

class ClampThread : public Thread {
public:
    enum RunType {
        BATCH,
        CONTINUOUS,
        ONCE
    };

    ClampThread(GlobalState& state_, VoltageClampWidget** voltageWidget_, CurrentClampWidget** currentWidget_, bool* voltageClampMode_, unsigned int unit_);
    ~ClampThread();

    void run() override;
    void done() override;

    void setRunType(RunType value) { runType = value; }

    void startRunning();
    void runWithType(RunType runType);
    void endRunning();

protected:
    RunType runType;
    GlobalState& state;
    CLAMP::Board& board;
    CLAMP::ClampConfig::ChipChannelList channelList;
    VoltageClampWidget** voltageWidget;
	CurrentClampWidget** currentWidget;
	bool voltageClampMode[CLAMP::MAX_NUM_CHIPS];
    CapacitiveCompensationController* capControlWidget;
	FeedbackBandwidthWidget* feedback; // Controls feedback resistor and capacitor

	unsigned int unit;
    CLAMP::SimplifiedWaveform simplifiedWaveform[CLAMP::MAX_NUM_CHIPS];

    void createWaveform();

    virtual void clear(bool filtersToo);
    void setCapacitiveCompensationImmediate();
    void readAndProcessOneCycle(bool first = true, double time = -1);
    void finishLastCycle();

    const std::vector<double>& getValues(unsigned int headstage);
	const std::vector<double>& getClampValues(unsigned int headstage);
	void setRCImmediate();

protected:
    void runContinuously();
    void runSemicontinuously();
    void runBatches();
    void runOnce();
    void setScaleImmediate();
    bool scaleChanged() const;

    bool capCompensationChanged() const;
    double capCompensationMagnitude;
    bool capCompensationConnect;

private:
	double bandwidth;
	CLAMP::Registers::Register3::Resistance resistance;
	double voltageOffset;
	std::vector<double> offsetVoltages;
	Controller* controlWidget;

	void switchToVoltageClamp(const CLAMP::ClampConfig::ChipChannel& channel, int holdingVoltage);
	void switchToCurrentClamp(const CLAMP::ClampConfig::ChipChannel& channel, CLAMP::ClampConfig::CurrentScale scale, int holdingCurrent);
	void clearOffsetVoltages();
};

