#-------------------------------------------------
#
# Project created by QtCreator 2012-02-21T22:22:50
#
#-------------------------------------------------

QT       += core gui network

TARGET = OpenNIC
TEMPLATE = app

SOURCES += main.cpp\
		opennic.cpp \
		opennicresolver.cpp \
		opennictest.cpp \
		opennicdns.cpp

HEADERS  += opennic.h \
		opennicresolver.h \
		opennictest.h \
		opennicdns.h

FORMS    += opennic.ui \
		settings.ui

RESOURCES += \
		opennic.qrc

win32:LIBS += C:\QtSDK\mingw\lib\libwsock32.a


