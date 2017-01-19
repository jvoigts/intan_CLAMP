#include "ExternalCommandWidget.h"
#include <QtGui>
#include "common.h"
#include "SaveFile.h"
#include "globalconstants.h"
#include "Board.h"

using namespace CLAMP;

//--------------------------------------------------------------------------
ExternalCommandWidget::ExternalCommandWidget(CLAMP::Board& board_, int unit_) :
	board(board_),
	unit(unit_)
{
	enableExternalCommand = new QCheckBox("Add External Command Signal", this);
	connect(enableExternalCommand, SIGNAL(toggled(bool)), this, SLOT(enableCommandChanged(bool)));

	commandSource = new QComboBox();
	commandSource->addItem(tr("ANALOG IN 1"));
	commandSource->addItem(tr("ANALOG IN 2"));
	if (board.expanderBoardPresent()) {
		commandSource->addItem(tr("ANALOG IN 3"));
		commandSource->addItem(tr("ANALOG IN 4"));
		commandSource->addItem(tr("ANALOG IN 5"));
		commandSource->addItem(tr("ANALOG IN 6"));
		commandSource->addItem(tr("ANALOG IN 7"));
		commandSource->addItem(tr("ANALOG IN 8"));
	}

	connect(commandSource, SIGNAL(currentIndexChanged(int)), this, SLOT(setCommandSource(int)));

	voltageCommandSensitivity = new QComboBox();
	voltageCommandSensitivity->addItem(tr("100 mV/V"));
	voltageCommandSensitivity->addItem(tr("1 V/V"));
	connect(voltageCommandSensitivity, SIGNAL(currentIndexChanged(int)), this, SLOT(setVoltageSensitivity(int)));
	voltageCommandSensitivity->setCurrentIndex(1);
	voltageCommandSensitivity->setCurrentIndex(0);  // force currentIndexChanged signal

	currentCommandSensitivity = new QComboBox();
	currentCommandSensitivity->addItem(tr("1 step/100 mV"));
	currentCommandSensitivity->addItem(tr("1 step/10 mV"));
	connect(currentCommandSensitivity, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentSensitivity(int)));
	currentCommandSensitivity->setCurrentIndex(1);
	currentCommandSensitivity->setCurrentIndex(0);  // force currentIndexChanged signal

	QHBoxLayout *hLayout1 = new QHBoxLayout();
	hLayout1->addWidget(new QLabel(tr(" V Clamp Sensitivity ")));
	hLayout1->addStretch(1);
	hLayout1->addWidget(voltageCommandSensitivity);

	QHBoxLayout *hLayout2 = new QHBoxLayout();
	hLayout2->addWidget(new QLabel(tr(" I Clamp Sensitivity ")));
	hLayout2->addStretch(1);
	hLayout2->addWidget(currentCommandSensitivity);

	QVBoxLayout *vLayout1 = new QVBoxLayout();
	vLayout1->addWidget(new QLabel(tr("Source")));
	vLayout1->addWidget(commandSource);
//	vLayout1->addStretch(1);

	QVBoxLayout *vLayout2 = new QVBoxLayout();
	vLayout2->addLayout(hLayout1);
	vLayout2->addStretch(1);
	vLayout2->addLayout(hLayout2);

	QHBoxLayout *hLayout4 = new QHBoxLayout();
	hLayout4->addLayout(vLayout1);
	hLayout4->addStretch(1);
	hLayout4->addLayout(vLayout2);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(enableExternalCommand);
	layout->addItem(hLayout4);
	if (!board.expanderBoardPresent()) {
		layout->addWidget(new QLabel(tr("Add Intan I/O Expander for 6 additional ANALOG IN ports.")));
	}
	layout->addStretch(1);

	setLayout(layout);
}

void ExternalCommandWidget::enableCommandChanged(bool state) {
	board.enableAdcControl(state, unit, 0);
}

void ExternalCommandWidget::setCommandSource(int adc) {
	board.selectAdcControl(commandSource->currentIndex(), unit, 0);
}

void ExternalCommandWidget::setVoltageSensitivity(int index) {
	switch (index) {
	case 0:
		board.setVoltageMultiplier(unit, 0.1);
		break;
	case 1:
		board.setVoltageMultiplier(unit, 1.0);

		break;
	}
}

void ExternalCommandWidget::setCurrentSensitivity(int index) {
	switch (index) {
	case 0:
		board.setCurrentMultiplier(unit, 825); // = 1 step/100 mV
		break;
	case 1:
		board.setCurrentMultiplier(unit, 8250); // = 1 step/10 mV
		break;
	}
}