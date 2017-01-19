#include "MarkerOutputWidget.h"
#include <QtGui>
#include "common.h"
#include "SaveFile.h"
#include "globalconstants.h"
#include "Board.h"

using namespace CLAMP;
using namespace CLAMP::ClampConfig;

//--------------------------------------------------------------------------
MarkerOutputWidget::MarkerOutputWidget(CLAMP::Board& board_, int unit_) :
	board(board_),
	unit(unit_)
{
	enableMarkerOutput = new QCheckBox("Enable Clamp Digital Marker Output", this);
	connect(enableMarkerOutput, SIGNAL(clicked(bool)), this, SLOT(enableOutputChanged(bool)));

	outputDestination = new QComboBox();
	outputDestination->addItem(tr("DIGITAL OUT 1"));
	outputDestination->addItem(tr("DIGITAL OUT 2"));
	if (board.expanderBoardPresent()) {
		outputDestination->addItem(tr("DIGITAL OUT 3"));
		outputDestination->addItem(tr("DIGITAL OUT 4"));
		outputDestination->addItem(tr("DIGITAL OUT 5"));
		outputDestination->addItem(tr("DIGITAL OUT 6"));
		outputDestination->addItem(tr("DIGITAL OUT 7"));
		outputDestination->addItem(tr("DIGITAL OUT 8"));
		outputDestination->addItem(tr("DIGITAL OUT 9"));
		outputDestination->addItem(tr("DIGITAL OUT 10"));
		outputDestination->addItem(tr("DIGITAL OUT 11"));
		outputDestination->addItem(tr("DIGITAL OUT 12"));
		outputDestination->addItem(tr("DIGITAL OUT 13"));
		outputDestination->addItem(tr("DIGITAL OUT 14"));
		outputDestination->addItem(tr("DIGITAL OUT 15"));
		outputDestination->addItem(tr("DIGITAL OUT 16"));
	}
	prevOutputDestination = outputDestination->currentIndex();
	connect(outputDestination, SIGNAL(currentIndexChanged(int)), this, SLOT(setOutputDestination(int)));

	QHBoxLayout *hLayout1 = new QHBoxLayout();
//	hLayout1->addWidget(new QLabel(tr("Output")));
	hLayout1->addWidget(outputDestination);
	hLayout1->addStretch(1);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(new QLabel(tr("Clamp marker is on MARK OUT.  Extra port may be selected here.")));
	layout->addWidget(enableMarkerOutput);
	layout->addItem(hLayout1);
	if (!board.expanderBoardPresent()) {
		layout->addWidget(new QLabel(tr("Add Intan I/O Expander for 14 additional DIGITAL OUT ports.")));
	}
	layout->addStretch(1);

	setLayout(layout);
}

void MarkerOutputWidget::enableOutputChanged(bool state) {
	board.enableDigitalMarker(unit, state);
}

void MarkerOutputWidget::setOutputDestination(int digOut) {
	board.setDigitalMarkerDestination(unit, digOut);
}

