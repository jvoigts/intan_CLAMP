//----------------------------------------------------------------------------------
// main.cpp
//
// Intan Technoloies CLAMP Interface
//
// Copyright (c) 2016 Intan Technologies LLC
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any applications that
// use Intan Technologies integrated circuits, and to alter it and redistribute it
// freely.
//
// See http://www.intantech.com for documentation and product information.
//----------------------------------------------------------------------------------

//#include <QtGui>
#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <stdexcept>
#include <iostream>
#include <memory>

#include "DisplayWindow.h"
#include "ControlWindow.h"
#include "guicon.h"
#include "common.h"
#include "Constants.h"
#include "Board.h"
#include <sstream>
#include <QDesktopWidget>
#include <QFile>
#include <QDir>

//#include <vld.h>

using std::exception;
using std::runtime_error;
using std::unique_ptr;
using CLAMP::Board;
using CLAMP::Registers::Register8;
using namespace CLAMP::ClampConfig;
using std::ostream;
using std::string;
using std::ostringstream;

bool logTemperature = true;

// Attach to board and do calibration
string setupBoard(QSplashScreen* splash, Board& board) {
    Qt::Alignment position = Qt::AlignCenter | Qt::AlignBottom;
    try {
        splash->showMessage(QObject::tr("Connecting to Intan CLAMP Controller..."), position, Qt::black);
		if (!board.open()) {
			//QMessageBox::critical(nullptr, "Intan CLAMP Controller Not Found",
			//	"Intan Technologies CLAMP Controller not found on any USB port.  "
			//		"<p>To use this application, connect the Intan CLAMP Controller "
			//		"to a USB port, then restart the application."
			//		"<p>Visit http://www.intantech.com for more information.");
			throw runtime_error("Intan Technologies CLAMP Controller not found on any USB port.");
		}

		uint8_t spiLedByte = 0;
		for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
			if (board.chip[i]->present) {
				spiLedByte += (1 << i);
			}
		}
		if (spiLedByte == 0) {
            throw runtime_error("No Intan CLAMP headstages detected.  Connect headstage(s) and restart.");
        }
		board.setSpiPortLeds(spiLedByte);
		board.setStatusLeds(true, 1);

        ostringstream oss;
        ostream* oldLogger = SetLogger(&oss);
		ChipChannelList channelList = board.getPresentChannels();
        splash->showMessage(QObject::tr("Calibrating voltage amplifiers..."), position, Qt::black);
        board.controller.voltageAmplifier.calibrate(channelList);
        splash->showMessage(QObject::tr("Calibrating difference amplifiers..."), position, Qt::black);
        board.controller.differenceAmplifier.calibrate1(channelList);
        splash->showMessage(QObject::tr("Calibrating voltage clamp DACs..."), position, Qt::black);
        board.controller.clampVoltageGenerator.calibrate(channelList);
        // splash->showMessage(QObject::tr("Calibrating difference amplifiers..."), position, Qt::black);
        board.controller.differenceAmplifier.calibrate2(channelList);
        splash->showMessage(QObject::tr("Calibrating current-to-voltage converters..."), position, Qt::black);
        board.controller.currentToVoltageConverter.calibrate(channelList);
        splash->showMessage(QObject::tr("Calibrating current clamp DACs..."), position, Qt::black);
        board.controller.clampCurrentGenerator.calibrate(channelList);

		board.controller.clampVoltageGenerator.setClampStepSizeImmediate(channelList, false);

        if (logTemperature) {
            LOG(true) << "Temperature:\t" << board.controller.temperatureSensor.readTemperature(0) << "C\n\n";
        }

        SetLogger(oldLogger);
        return oss.str();
    }
    catch (exception& e) {
        QMessageBox::critical(splash, QObject::tr("Error"), e.what());
        exit(EXIT_FAILURE); // abort application
    }

}

void center(QWidget& left, QWidget& right) {
    // Now center the window on the desktop
    QRect rec = QApplication::desktop()->screenGeometry();
    int xSpacing = (rec.width() - left.width() - right.width()) / 3;
    left.move(xSpacing, (rec.height() - left.height()) / 2);
    right.move(xSpacing + left.width() + xSpacing, (rec.height() - right.height()) / 2);
}

void copyResource(QFile& resultFile, QString resourceName) {
    if (resultFile.open(QIODevice::ReadWrite))
    {
        QFile resourceFile(resourceName);
        if (resourceFile.open(QIODevice::ReadOnly))
        {
            resultFile.write(resourceFile.readAll());
            resourceFile.close();
        }
        resultFile.close();
    }
}

// Starts application main window.
int main(int argc, char *argv[]) {
    try {
        RedirectIOToConsole();
        SetLogger(&std::cerr);

        QApplication app(argc, argv);

        QSplashScreen* splash = new QSplashScreen();
        splash->setPixmap(QPixmap(":/images/splash.png"));
        splash->show();

        Qt::Alignment topRight = Qt::AlignRight | Qt::AlignTop;
        splash->showMessage(QObject::tr("Starting..."), topRight, Qt::black);

        unique_ptr<Board> board(new Board());

        string calibrationReport = setupBoard(splash, *board);

		board->enableChannels({}, true);
		board->clearCommands();

		ChipChannelList channelList = board->getPresentChannels();

        board->controller.offChipComponents.setInputImmediate(channelList, Register8::ElectrodePin);

        GlobalState state(board);
        DisplayWindow display(state, calibrationReport, channelList.front().chip);
        ControlWindow control(&display, state);

		for (int i = 0; i < CLAMP::MAX_NUM_CHIPS; i++) {
			state.datastore[i].displayWindow = &display;
			state.datastore[i].controlWindow = &control;

			// Set the clamp tab to Voltage Clamp mode.
			control.tabWidget[i]->setCurrentIndex(1); // HACK: set it to something else first, to be sure to emit an event when we come back to it
			control.tabWidget[i]->setCurrentIndex(0);
		}

        control.show();
        display.show();
        center(display, control);

        splash->showMessage(QObject::tr("All done"), topRight, Qt::black);
        splash->finish(&display);
        delete splash;

        // QSound cannot play wav files directly from Qt resource files, so
        // this is a workaround:  Copy the wav file from the resource file to
        // a temporary directory, then play it from there when needed.
        QFile fileTemp(QDir::tempPath() + "/beep.wav");
        copyResource(fileTemp, ":/sounds/beep.wav");

        // QFile fileTemp2(QDir::tempPath() + "/buzz.wav");
        // copyResource(fileTemp2, ":/sounds/buzz.wav");

		LOG(true) << endl << "====================================" << endl;

        int retVal = app.exec();

        fileTemp.remove();
        // fileTemp2.remove();

        return retVal;
    }
    catch (exception& e) {
        QMessageBox::critical(nullptr, "Error", e.what());
        return -1;
    }
}
