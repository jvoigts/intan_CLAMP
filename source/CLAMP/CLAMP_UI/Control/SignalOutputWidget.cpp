#include "SignalOutputWidget.h"
#include <QtGui>
#include "common.h"
#include "SaveFile.h"
#include "globalconstants.h"
#include "Board.h"

using namespace CLAMP;
using namespace CLAMP::ClampConfig;

//--------------------------------------------------------------------------
SignalOutputWidget::SignalOutputWidget(CLAMP::Board& board_, int unit_, bool clampOutput_) :
	board(board_),
	unit(unit_),
	clampOutput(clampOutput_)
{
	if (clampOutput) {
		enableSignalOutput = new QCheckBox("Enable Clamp Command Monitor", this);
	}
	else {
		enableSignalOutput = new QCheckBox("Enable Signal Output", this);
	}
	connect(enableSignalOutput, SIGNAL(clicked(bool)), this, SLOT(enableOutputChanged(bool)));

	outputDestination = new QComboBox();
	outputDestination->addItem(tr("ANALOG OUT 1 (AUDIO L)"));
	outputDestination->addItem(tr("ANALOG OUT 2 (AUDIO R)"));
	if (board.expanderBoardPresent()) {
		outputDestination->addItem(tr("ANALOG OUT 3"));
		outputDestination->addItem(tr("ANALOG OUT 4"));
		outputDestination->addItem(tr("ANALOG OUT 5"));
		outputDestination->addItem(tr("ANALOG OUT 6"));
		outputDestination->addItem(tr("ANALOG OUT 7"));
		outputDestination->addItem(tr("ANALOG OUT 8"));
	}
	prevOutputDestination = outputDestination->currentIndex();
	connect(outputDestination, SIGNAL(currentIndexChanged(int)), this, SLOT(setOutputDestination(int)));

	outputVoltageScale = new QComboBox();
	if (clampOutput) {
		outputVoltageScale->addItem(tr("10 Vclamp"));
		outputVoltageScale->addItem(tr("1 Vclamp"));
	}
	else {
		outputVoltageScale->addItem(tr("10 Vm"));
		outputVoltageScale->addItem(tr("1 Vm"));
	}
	connect(outputVoltageScale, SIGNAL(currentIndexChanged(int)), this, SLOT(setVoltageScale(int)));
	outputVoltageScale->setCurrentIndex(1);
	outputVoltageScale->setCurrentIndex(0);  // force currentIndexChanged signal

	outputCurrentScale = new QComboBox();
	if (clampOutput) {
		outputCurrentScale->addItem(tr("100 mV/step"));
		outputCurrentScale->addItem(tr("10 mV/step"));
	}
	else {
		outputCurrentScale->addItem(tr("1 V/nA"));
		outputCurrentScale->addItem(tr("100 mV/nA"));
	}
	connect(outputCurrentScale, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentScale(int)));
	outputCurrentScale->setCurrentIndex(1);
	outputCurrentScale->setCurrentIndex(0);  // force currentIndexChanged signal

	outputCurrentScaleLabel = new QLabel("label");
	outputCurrentScaleLabel->hide();

	QHBoxLayout *hLayout1 = new QHBoxLayout();
	if (clampOutput) {
		hLayout1->addWidget(new QLabel(tr(" Vclamp Scale ")));
	}
	else {
		hLayout1->addWidget(new QLabel(tr(" Vm Scale ")));
	}
	hLayout1->addStretch(1);
	hLayout1->addWidget(outputVoltageScale);

	QHBoxLayout *hLayout2 = new QHBoxLayout();
	if (clampOutput) {
		hLayout2->addWidget(new QLabel(tr(" Iclamp Scale ")));
	}
	else {
		hLayout2->addWidget(new QLabel(tr(" I Scale ")));
	}
	hLayout2->addStretch(1);
	hLayout2->addWidget(outputCurrentScale);
	hLayout2->addWidget(outputCurrentScaleLabel);

	QVBoxLayout *vLayout1 = new QVBoxLayout();
	vLayout1->addWidget(new QLabel(tr("Output")));
	vLayout1->addWidget(outputDestination);
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
	layout->addWidget(enableSignalOutput);
	layout->addItem(hLayout4);
	if (!board.expanderBoardPresent()) {
		layout->addWidget(new QLabel(tr("Add Intan I/O Expander for 6 additional ANALOG OUT ports.")));
	}
	layout->addStretch(1);

	setLayout(layout);

	updateFeedbackResistance(3);  // TODO: tie to actual value of FeedbackBandwidthWidget currentScale QComboBox.
}

void SignalOutputWidget::enableOutputChanged(bool state) {
	if (state) {
		ChipChannel chipChannel;
		bool outputClamp;
		int dac = outputDestination->currentIndex();
		if (board.isDacInUse(dac, chipChannel, outputClamp)) {
			enableSignalOutput->setCheckState(Qt::Unchecked);
			DacInUseDialog dialog(this, dac, chipChannel.chip, outputClamp);
			dialog.exec();
		}
		else {
			board.configureDac(outputDestination->currentIndex(), true, unit, 0, clampOutput);
			setVoltageScale(outputVoltageScale->currentIndex());
			setCurrentScale(outputCurrentScale->currentIndex());
		}
	}
	else {
		board.configureDac(outputDestination->currentIndex(), false, unit, 0, clampOutput);
		setVoltageScale(outputVoltageScale->currentIndex());
		setCurrentScale(outputCurrentScale->currentIndex());
	}
}

void SignalOutputWidget::setOutputDestination(int dac) {
	if (enableSignalOutput->isChecked()) {
		ChipChannel chipChannel;
		bool outputClamp;
		if (board.isDacInUse(dac, chipChannel, outputClamp)) {
			if (chipChannel.chip != unit || outputClamp != clampOutput) {
				outputDestination->setCurrentIndex(prevOutputDestination);
				DacInUseDialog dialog(this, dac, chipChannel.chip, outputClamp);
				dialog.exec();
			}
		}
		else {
			board.configureDac(prevOutputDestination, false, unit, 0, clampOutput);
			board.configureDac(dac, true, unit, 0, clampOutput);
			setVoltageScale(outputVoltageScale->currentIndex());
			setCurrentScale(outputCurrentScale->currentIndex());
			prevOutputDestination = outputDestination->currentIndex();
		}
	}
	else {
		prevOutputDestination = outputDestination->currentIndex();
	}
}

void SignalOutputWidget::setVoltageScale(int index) {
	if (clampOutput) {
		switch (index) {
		case 0:
			board.setDacVoltageMultiplier(outputDestination->currentIndex(), 10.0); // 10 V/V
			break;
		case 1:
			board.setDacVoltageMultiplier(outputDestination->currentIndex(), 1.0); // 1 V/V
			break;
		}
	}
	else {
		switch (index) {
		case 0:
			board.setDacVoltageMultiplier(outputDestination->currentIndex(), 2.56); // 10 V/V
			board.setDacVoltageOffset(outputDestination->currentIndex(), -1 * board.chip[unit]->channel[0]->voltageAmpResidual); 
			break;
		case 1:
			board.setDacVoltageMultiplier(outputDestination->currentIndex(), 0.256); // 1 V/V
			board.setDacVoltageOffset(outputDestination->currentIndex(), -1 * board.chip[unit]->channel[0]->voltageAmpResidual);
			break;
		}
	}
}

void SignalOutputWidget::setCurrentScale(int index) {
	if (clampOutput) {
		switch (index) {
		case 0:
			board.setDacCurrentMultiplier(outputDestination->currentIndex(), 100.0); // 100 mV/step
			break;
		case 1:
			board.setDacCurrentMultiplier(outputDestination->currentIndex(), 10.0); // 10 mV/step
			break;
		}
	}
	else {
		switch (index) {
		case 0:
			board.setDacCurrentMultiplier(outputDestination->currentIndex(), 1.024e9 / currentFeedbackResistance); // 1 V/nA
			board.setDacCurrentOffset(outputDestination->currentIndex(), -1 * board.chip[unit]->channel[0]->differenceAmpResidual);
			break;
		case 1:
			board.setDacCurrentMultiplier(outputDestination->currentIndex(), 1.024e8 / currentFeedbackResistance); // 100 mV/nA
			board.setDacCurrentOffset(outputDestination->currentIndex(), -1 * board.chip[unit]->channel[0]->differenceAmpResidual);
			break;
		}
	}
}

void SignalOutputWidget::setCurrentFeedbackResistance(int index) {
	currentFeedbackResistance = board.chip[unit]->channel[0]->rFeedback[index + 1];
}

void SignalOutputWidget::updateFeedbackResistance(int index) {
	setCurrentFeedbackResistance(index);

	if (index == 0) {
		outputCurrentScale->hide();
		outputCurrentScaleLabel->setText("10 mV/nA");
		outputCurrentScaleLabel->show();
		board.setDacCurrentMultiplier(outputDestination->currentIndex(), 1.024e7 / currentFeedbackResistance); // 10 mV/nA
		board.setDacCurrentOffset(outputDestination->currentIndex(), -1 * board.chip[unit]->channel[0]->differenceAmpResidual);
	}
	else if (index == 1) {
		outputCurrentScale->hide();
		outputCurrentScaleLabel->setText("100 mV/nA");
		outputCurrentScaleLabel->show();
		board.setDacCurrentMultiplier(outputDestination->currentIndex(), 1.024e8 / currentFeedbackResistance); // 100 mV/nA
		board.setDacCurrentOffset(outputDestination->currentIndex(), -1 * board.chip[unit]->channel[0]->differenceAmpResidual);
	}
	else {
		outputCurrentScale->show();
		outputCurrentScaleLabel->hide();
		setCurrentScale(outputCurrentScale->currentIndex());
	}
}

//--------------------------------------------------------------------------
DacInUseDialog::DacInUseDialog(QWidget* parent, int dac, int blocker, bool outputClamp) :
	QDialog(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	QString mode = outputClamp ? "clamp output" : "signal output";

	layout->addWidget(new QLabel(QString("ANALOG OUT %1 is currently being used by Port ").arg(dac + 1) +
		QString(QChar(65 + blocker)) + QString(" for ") + mode + "."));

	layout->addStretch(1);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	layout->addWidget(buttonBox);

	setLayout(layout);
}
