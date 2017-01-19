#pragma once

#include <QWidget>
#include <QDialog>
#include "Board.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QString;
class QDoubleSpinBox;

class MarkerOutputWidget : public QWidget {
    Q_OBJECT

public:
	MarkerOutputWidget(CLAMP::Board& board_, int unit_);
	
private slots:
	void enableOutputChanged(bool state);
	void setOutputDestination(int digOut);

private:
	CLAMP::Board& board;
	int unit;
	int prevOutputDestination;

	QCheckBox* enableMarkerOutput;
    QComboBox* outputDestination;
};

