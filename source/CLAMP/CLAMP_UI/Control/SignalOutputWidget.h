#pragma once

#include <QWidget>
#include <QDialog>
#include "Board.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QString;
class QDoubleSpinBox;

class SignalOutputWidget : public QWidget {
    Q_OBJECT

public:
	SignalOutputWidget(CLAMP::Board& board_, int unit_, bool clampOutput_);
	
public slots:
	void updateFeedbackResistance(int index);

private slots:
	void enableOutputChanged(bool state);
	void setOutputDestination(int dac);
	void setVoltageScale(int index);
	void setCurrentScale(int index);

private:
	CLAMP::Board& board;
	int unit;
	int prevOutputDestination;
	bool clampOutput;

	QCheckBox* enableSignalOutput;
    QComboBox* outputDestination;
	QComboBox* outputVoltageScale;
	QComboBox* outputCurrentScale;
	QLabel* outputCurrentScaleLabel;

	void setCurrentFeedbackResistance(int index);
	double currentFeedbackResistance;
};

class DacInUseDialog : public QDialog {
	Q_OBJECT

public:
	DacInUseDialog(QWidget* parent, int dac, int blocker, bool outputClamp);
};
