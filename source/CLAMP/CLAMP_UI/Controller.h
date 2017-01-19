#pragma once

namespace CLAMP {
    class SimplifiedWaveform;
}

class Controller {
public:
    virtual ~Controller() {}

    virtual int getHoldingValue() = 0;
    virtual CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, bool holdingOnly = false, unsigned int lastIndex = 0) = 0;
    virtual unsigned int getCurrentScale() { return 0; }
    virtual void startMessage(int unit) = 0;
    virtual void endMessage(int unit) = 0;
};

class CapacitiveCompensationController {
public:
    virtual ~CapacitiveCompensationController() {}

    virtual double getCapCompensationValue() const = 0;
};
