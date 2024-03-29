# This file is a part of OpenNIC Wizard
# Copywrong (c) 2012-2022 Mike Sharkey
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 1776):
# <mike@8bitgeek.net> wrote this file.
# As long as you retain this notice you can do whatever you want with this
# stuff. If we meet some day, and you think this stuff is worth it,
# you can buy me a beer in return. ~ Mike Sharkey
# ----------------------------------------------------------------------------

QT       += core gui network widgets

TARGET = opennicwizui
TEMPLATE = app
CONFIG += warn_on \
	thread \
	qt \
	debug

SOURCES += main.cpp\
        opennic.cpp \
        opennicqueryhistorydialog.cpp

INCLUDEPATH += ../common

HEADERS  += opennic.h \
	opennicqueryhistorydialog.h


FORMS    += opennic.ui \
        settings.ui \
        queries.ui

RESOURCES += \
        opennic.qrc

unix:LIBS += ../common/libcommon.a
win32:LIBS += ../common/debug/libcommon.a
#win32:LIBS += /cygdrive/c/cygwin64/lib/w32api/libwsock32.a
#win32:LIBS += -L/cygdrive/c/Qt/Tools/mingw810_64/x86_64-w64-mingw32/lib/
