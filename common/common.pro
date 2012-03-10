#-------------------------------------------------
#-------------------------------------------------

QT       += core network

TARGET = libcommon.a
TEMPLATE = lib
CONFIG += warn_on \
	thread \
	qt \
	debug

SOURCES += \ 
    opennicnet.cpp \
    opennicpacket.cpp

HEADERS  += \ 
    opennicnet.h \
    opennicpacket.h

FORMS    += 

RESOURCES += 
