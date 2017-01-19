#pragma once

#include <QCheckBox>
#include <QGroupBox>
#include "Thread.h"
#include "MVC.h"
#include "SimplifiedWaveform.h"
#include <string>
#include <QDialog>

class GlobalState;
class QLabel;
class Controller;
class CapacitiveCompensationController;
class FeedbackBandwidthWidget;
class QSound;

class ZapParams : public QObject {
    Q_OBJECT

public:
    StepDoubleHolder amplitudeV;
    StepDoubleHolder durationMs;

    ZapParams(double amplitude, double duration);

    void set(const ZapParams& other);
    CLAMP::SimplifiedWaveform getSimplifiedWaveform(double samplingRate, int holdingValue);
    std::string toHtml();

signals:
    void valuesChanged();

private slots:
    void notifyObserver();
};

class ZapParamsDisplay : public QWidget {
    Q_OBJECT

public:
    ZapParamsDisplay(ZapParams& params_);

public slots:
    void redoLayout();

protected:
    void mousePressEvent(QMouseEvent *ev) override;

private:
    ZapParams& params;

    QLabel* label;
};

class ZapDialog : public QDialog {
    Q_OBJECT

public:
    ZapDialog(QWidget* parent, ZapParams& params_);

private:
    ZapParams& params;
};



class ZapWidget : public QGroupBox {
    Q_OBJECT

public:
    ZapWidget(GlobalState& state_, Controller& controller_, FeedbackBandwidthWidget& feedback_, int unit_);
	void setControlsEnabled(bool enabled);

private slots:
    void doZap();

private:
    GlobalState& state;
    Controller& controller;
    FeedbackBandwidthWidget& feedback;
	QPushButton* button;
	QCheckBox* cb;

    ZapParams params;
    ZapParamsDisplay* paramsDisplay;

    BoolHolder zapEnabled;
	int unit;
};


class ZapThread : public Thread {
public:
    ZapThread(GlobalState& state_, ZapParams& params_, Controller& controller_, FeedbackBandwidthWidget& feedback_, int unit_);
    ~ZapThread();

    void run() override;
    void done() override;

private:
    GlobalState& state;
    ZapParams& params;
    Controller& controller;
    FeedbackBandwidthWidget& feedback;
    QSound* beep;
	int unit;
};