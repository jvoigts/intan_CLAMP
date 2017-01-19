INCLUDEPATH += $$PWD

unix:QMAKE_CXXFLAGS += -std=c++11

HEADERS       += \
    $$PWD/common.h \
    $$PWD/streams.h \
    $$PWD/guicon.h

SOURCES += \
    $$PWD/common.cpp \
    $$PWD/streams.cpp \
    $$PWD/guicon.cpp
    
