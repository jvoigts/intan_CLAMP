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
class QRadioButton;
class CapacitiveCompensationController;
class QSound;

class BuzzParams : public QObject {
    Q_OBJECT

public:
    BoolHolder useLarge;
    StepDoubleHolder durationMs;

    BuzzParams(bool largeAmplitude, double duration);

    void set(const BuzzParams& other);
    std::string toHtml();

signals:
    void valuesChanged();

private slots:
    void notifyObserver();
};

class BuzzParamsDisplay : public QWidget {
    Q_OBJECT

public:
    BuzzParamsDisplay(BuzzParams& params_);

public slots:
    void redoLayout();

protected:
    void mousePressEvent(QMouseEvent *ev) override;

private:
    BuzzParams& params;

    QLabel* label;
};

class BuzzDialog : public QDialog {
    Q_OBJECT

public:
    BuzzDialog(QWidget* parent, BuzzParams& params_);

private:
    BuzzParams& params;
    QRadioButton* largeButton;
    QRadioButton* smallButton;
};



class BuzzWidget : public QGroupBox {
    Q_OBJECT

public:
    BuzzWidget(GlobalState& state_, Controller& controller_, CapacitiveCompensationController& cap_, int unit_);
	void setControlsEnabled(bool enabled);

private slots:
    void doBuzz();

private:
    GlobalState& state;
    Controller& controller;
    CapacitiveCompensationController& cap;

	QPushButton* button;
	QCheckBox* cb;

    BuzzParams params;
    BuzzParamsDisplay* paramsDisplay;

    BoolHolder buzzEnabled;
	int unit;
};


class BuzzThread : public Thread {
public:
    BuzzThread(GlobalState& state_, Controller& controller_, BuzzParams& params_, CapacitiveCompensationController& cap_, int unit_);
    ~BuzzThread();

    void run() override;
    void done() override;

private:
    GlobalState& state;
    Controller& controller;
    CapacitiveCompensationController& cap;
    BuzzParams& params;
    QSound* sound;
	int unit;
};