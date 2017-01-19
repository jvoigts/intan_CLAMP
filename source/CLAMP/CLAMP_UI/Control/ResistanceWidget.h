#pragma once

#include <QDialog>
#include "Controller.h"
#include "MVC.h"
#include <string>
#include "WaveformTimingWidget.h"

class QSpinBox;
class QDoubleSpinBox;
class GlobalState;
class HoldingVoltageWidget;
class QLabel;

class ResistanceParams : public QObject {
    Q_OBJECT

public:
    ResistanceParams();

    StepDoubleHolder amplitudeInMv;
    TimingParams timingParams;
    DoubleHolder intervalS;

    void set(const ResistanceParams& other);
    CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, int holdingValue, double pipetteOffset);
    std::string toHtml();

signals:
    void valuesChanged();

private slots:
    void notifyObserver();
};

class ResistanceParamsDisplay : public QWidget {
    Q_OBJECT

public:
    ResistanceParamsDisplay(ResistanceParams& params_);

public slots:
    void redoLayout();

protected:
    void mousePressEvent(QMouseEvent *ev) override;

private:
    ResistanceParams& params;

    QLabel* label;
};

class ResistanceDialog : public QDialog {
    Q_OBJECT

public:
    ResistanceDialog(QWidget* parent, ResistanceParams& params_);

private:
    ResistanceParams& params;
};
