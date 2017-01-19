#pragma once

#include <QWidget>
#include "Board.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QString;
class QDoubleSpinBox;

class ExternalCommandWidget : public QWidget {
    Q_OBJECT

public:
    ExternalCommandWidget(CLAMP::Board& board_, int unit_);
	
private slots:
	void enableCommandChanged(bool state);
	void setCommandSource(int adc);
	void setVoltageSensitivity(int index);
	void setCurrentSensitivity(int index);

private:
	CLAMP::Board& board;
	int unit;

	QCheckBox* enableExternalCommand;
    QComboBox* commandSource;
	QComboBox* voltageCommandSensitivity;
	QComboBox* currentCommandSensitivity;
};
