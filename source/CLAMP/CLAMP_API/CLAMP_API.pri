include ("../../OpalKelly/OpalKelly.pri")
include ("../../Common/Common.pri")

INCLUDEPATH += $$PWD

HEADERS       += \
    $$PWD/BesselFilter.h \
    $$PWD/Board.h \
    $$PWD/Channel.h \
    $$PWD/Chip.h \
    $$PWD/ChipProtocol.h \
    $$PWD/ClampController.h \
    $$PWD/Constants.h \
    $$PWD/DataAnalysis.h \
    $$PWD/OpalKellyBoard.h \
    $$PWD/OpalKellyLibraryHandle.h \
    $$PWD/RAM.h \
    $$PWD/ReadQueue.h \
    $$PWD/Registers.h \
    $$PWD/SaveFile.h \
    $$PWD/SimplifiedWaveform.h \
    $$PWD/Thread.h \
    $$PWD/USBPacket.h \
    $$PWD/Waveform.h \
    $$PWD/WaveformCommand.h

SOURCES += \
    $$PWD/BesselFilter.cpp \
    $$PWD/Board.cpp \
    $$PWD/Channel.cpp \
    $$PWD/Chip.cpp \
    $$PWD/ChipProtocol.cpp \
    $$PWD/ClampController.cpp \
    $$PWD/DataAnalysis.cpp \
    $$PWD/OpalKellyBoard.cpp \
    $$PWD/OpalKellyLibraryHandle.cpp \
    $$PWD/RAM.cpp \
    $$PWD/ReadQueue.cpp \
    $$PWD/Registers.cpp \
    $$PWD/SaveFile.cpp \
    $$PWD/SimplifiedWaveform.cpp \
    $$PWD/Thread.cpp \
    $$PWD/USBPacket.cpp \
    $$PWD/Waveform.cpp \
    $$PWD/WaveformCommand.cpp
    
linux-g++ {
	EXTRA_BINFILES += $$PWD/../FPGA/main.bit
	for(FILE, EXTRA_BINFILES){
		QMAKE_PRE_LINK += $$quote($(COPY_FILE) \"$${FILE}\" ./ $$escape_expand(\\n\\t))
	}
}

