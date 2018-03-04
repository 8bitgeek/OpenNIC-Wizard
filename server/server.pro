# This file is a part of OpenNIC Wizard
# Copywrong (c) 2012-2018 Mike Sharkey
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 1776):
# <mike@8bitgeek.net> wrote this file.
# As long as you retain this notice you can do whatever you want with this
# stuff. If we meet some day, and you think this stuff is worth it,
# you can buy me a beer in return. ~ Mike Sharkey
# ----------------------------------------------------------------------------

QT  += core network widgets script

TARGET = OpenNICServer
TEMPLATE = app
CONFIG += warn_on \
	thread \
	qt \
	debug

SOURCES += main.cpp\
        opennicserver.cpp \
        qtservice.cpp \
        opennicdnsquery.cpp \
        opennicresolverpool.cpp \
        opennicsystem.cpp \
        opennicdomainname.cpp \
        opennicdomainnamepool.cpp \
        opennicdnsquerylistener.cpp \
        opennicresolver.cpp \
        opennicsystem_win.cpp \
        opennicsystem_linux.cpp


win32::SOURCES += qtservice_win.cpp \
        opennicsystem_win.cpp

unix::SOURCES += qtservice_unix.cpp \
        qtunixserversocket.cpp \
        qtunixsocket.cpp \
        opennicsystem_linux.cpp

INCLUDEPATH += ../common

HEADERS  += opennicserver.h \
        qtservice.h \
        qtservice_p.h \
        qtunixserversocket.h \
        qtunixsocket.h \
        opennicdnsquery.h \
        opennicresolverpool.h \
        opennicsystem.h \
        opennicdomainname.h \
        opennicdomainnamepool.h \
        opennicdnsquerylistener.h \
        opennicresolver.h \
        opennicsystem_linux.h \
        opennicsystem_win.h

RESOURCES += \
		opennicserver.qrc

unix:LIBS += ../common/libcommon.a
win32:LIBS += ../common/debug/libcommon.a
win32:LIBS += C:/MinGW/lib/libwsock32.a
