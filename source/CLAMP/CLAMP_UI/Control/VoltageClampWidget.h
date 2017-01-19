#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QDialog>
#include "Controller.h"
#include "AppliedWaveformWidget.h"
#include "ResistanceWidget.h"

class QWidget;
class QSpinBox;
class QLabel;
class WaveformTimingWidget;
class WaveformAmplitudeWidget;
class QDoubleSpinBox;
class QRadioButton;
class GlobalState;
class NumCyclesWidget;
class HoldingVoltageWidget;
struct PlotConfiguration;
class ZapWidget;
class QCheckBox;

namespace CLAMP {
    class ClampController;
    namespace IO {
        struct VoltageClampSettings;
    }
}

class AppliedVoltageWaveformWidget : public QGroupBox {
    Q_OBJECT

public:
    AppliedVoltageWaveformWidget();
    CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, int holdingValue, double pipetteOffset, bool holdingOnly = false, unsigned int lastIndex = 0);
    std::string getDescription(const std::string& holdingString) const;
	void enableControls(bool enabled);

public:
    PulseTrainParams sealTestParams;
    PulseTrainParamsDisplay* sealTestParamsDisplay;

    ResistanceParams resistanceParams;
    ResistanceParamsDisplay* resistanceParamsDisplay;

    MultistepParams multistepParams;
    MultistepParamsDisplay* multistepParamsDisplay;

	ArbWaveformParamsDisplay* arbWaveformParamsDisplay;

    QRadioButton* holding;
    QRadioButton* sealTest;
    QRadioButton* resistance;
    QRadioButton* multistep;
	QRadioButton* arbWaveform;

private slots:
	void enableArbWaveformButton();
};

class FeedbackBandwidthWidget;
class PipetteOffsetWidget;
class VoltageClampWidget : public QWidget, public Controller {
    Q_OBJECT

public:
    VoltageClampWidget(GlobalState& state_, int unit_);

    FeedbackBandwidthWidget* feedback;
    ZapWidget* zap;
    PipetteOffsetWidget* pipetteOffset;

    int getHoldingValue() override;
    CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, bool holdingOnly = false, unsigned int lastIndex = 0) override;
    void startMessage(int unit) override;
    void endMessage(int unit) override;

    void fillWaveformSettings(CLAMP::IO::VoltageClampSettings& settings);
    void getDisplayConfig(PlotConfiguration& config);
	void setControlsEnabled(bool enabled);

signals:
    void changeDisplay();

public slots:
    void setWholeCell(double Ra, double Rm, double Cm);

private slots:
    void optionsChanged();

private:
    GlobalState& state;
    HoldingVoltageWidget* holdingVoltage;
	int unit;

    AppliedVoltageWaveformWidget* applied;

    QLabel* raValue;
    QLabel* rmValue;
    QLabel* cmValue;
    QGroupBox* cellParameters;
    void createCellParametersLayout();

    QCheckBox* vCellCorrection;
	QCheckBox* appliedPlusAdc;
    std::string holdingText();
};
