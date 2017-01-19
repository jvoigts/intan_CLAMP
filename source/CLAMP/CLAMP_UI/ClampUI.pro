include ("../CLAMP_API/CLAMP_API.pri")
include ("Control/Control.pri")
include ("Display/Display.pri")

unix:LIBS += -L./ -lokFrontPanel -ldl

TARGET = ClampUI

TEMPLATE = app

QT += widgets gui

CONFIG += static

SOURCES += \
    ClampThread.cpp \
    DataStore.cpp \
    GlobalState.cpp \
    GUIUtil.cpp \
    main.cpp

HEADERS += \
    ClampThread.h \
    Controller.h \
    DataStore.h \
    globalconstants.h \
    GlobalState.h \
    GUIUtil.h

RESOURCES = ClampUI.qrc

