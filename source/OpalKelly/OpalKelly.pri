INCLUDEPATH += $$PWD

HEADERS       += \
    $$PWD/okFrontPanelDLL.h

SOURCES += \
    $$PWD/okFrontPanelDLL.cpp

linux-g++ {
	EXTRA_BINFILES += $$PWD/"Opal Kelly library files/Linux 64-bit/libokFrontPanel.so"
	for(FILE, EXTRA_BINFILES){
		QMAKE_PRE_LINK += $$quote($(COPY_FILE) \"$${FILE}\" ./ $$escape_expand(\\n\\t) )
	}
}
    
