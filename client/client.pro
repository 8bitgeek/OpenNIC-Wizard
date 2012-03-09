#-------------------------------------------------
#
# Project created by QtCreator 2012-02-21T22:22:50
#
#-------------------------------------------------

QT       += core gui network

TARGET = OpenNIC
TEMPLATE = app
CONFIG += warn_on \
	thread \
	qt \
	uitools \
	debug

SOURCES += main.cpp\
		opennic.cpp

HEADERS  += opennic.h

FORMS    += opennic.ui \
		settings.ui \
    queries.ui

RESOURCES += \
		opennic.qrc

win32:LIBS += C:/QtSDK/mingw/lib/libwsock32.a
