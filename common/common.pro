#-------------------------------------------------

QT       += core network

TARGET = common
TEMPLATE = lib
CONFIG += staticlib \
	warn_on \
	thread \
	qt \
	debug

SOURCES += \ 
    opennicnet.cpp \
    opennicpacket.cpp

HEADERS  += \ 
    opennicnet.h \
    opennicpacket.h \
    opennicversion.h

FORMS    += 

RESOURCES += 
