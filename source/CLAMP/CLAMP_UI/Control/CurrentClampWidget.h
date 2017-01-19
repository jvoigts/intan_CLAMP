#pragma once

#include <QWidget>
#include "Controller.h"
#include "WaveformTimingWidget.h"
#include "AppliedWaveformWidget.h"
#include <string>

class QWidget;
class QSpinBox;
class WaveformTimingWidget;
class WaveformAmplitudeWidget;
class QDoubleSpinBox;
class GlobalState;
class NumCyclesWidget;
class QRadioButton;
struct PlotConfiguration;
class BuzzWidget;
class QCheckBox;

namespace CLAMP {
    class ClampController;
    namespace IO {
        struct CurrentClampSettings;
    }
}

class AppliedCurrentWaveformWidget : public QGroupBox {
    Q_OBJECT

public:
    AppliedCurrentWaveformWidget();
    CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, int holdingValue, bool holdingOnly = false, unsigned int lastIndex = 0);
    std::string getDescription(const std::string& holdingValue) const;
    void setStepIndex(unsigned int index);
	void setEnabled(bool enabled);

public:
    PulseTrainParams tuningParams;
    PulseTrainParamsDisplay* tuningParamsDisplay;

    MultistepParams waveformParams;
    MultistepParamsDisplay* waveformParamsDisplay;

	ArbWaveformParamsDisplay* arbWaveformParamsDisplay;

    QRadioButton* holding;
    QRadioButton* tuning;
    QRadioButton* multistep;
	QRadioButton* arbWaveform;

private slots:
	void enableArbWaveformButton();

private:
    unsigned int index;
};

class QComboBox;
class CurrentClampWidget : public QWidget, public Controller {
    Q_OBJECT

public:
    CurrentClampWidget(GlobalState& state_, CapacitiveCompensationController& cap, int unit_);

    int getHoldingValue() override;
    CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, bool holdingOnly = false, unsigned int lastIndex = 0) override;
    void startMessage(int unit) override;
    void endMessage(int unit) override;

    void fillWaveformSettings(CLAMP::IO::CurrentClampSettings& settings);
    void getDisplayConfig(PlotConfiguration& config);
    unsigned int getCurrentScale() override;
	void setControlsEnabled(bool enabled);

    QComboBox *currentStepSizeComboBox;
    BuzzWidget* buzz;
    QPushButton* zeroCurrentButton;

signals:
    void changeDisplay();

private slots:
    void setCurrentStepSize(int index);
    void setZeroCurrent();
    void optionsChanged();
    void setWholeCell(double Ra, double Rm, double Cm);

private:
    GlobalState& state;
    CapacitiveCompensationController& capController;
	int unit;

    QDoubleSpinBox *holdingAmplitudeSpinBox;
	QDoubleSpinBox *seriesResistanceSpinBox;
	QWidget* createHoldingWidget();

    AppliedCurrentWaveformWidget* applied;

    QWidget* createCurrentScaleWidget();
    QCheckBox* bridgeBalance;
	QCheckBox* appliedPlusAdc;
    std::string holdingText();
};
