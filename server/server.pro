#    OpenNIC Wizard
#    Copyright (C) 2012-2015  Mike Sharkey <mike@8bitgeek.net>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


QT  += core network script uitools widgets

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
    opennicresolver.cpp


win32::SOURCES += qtservice_win.cpp

unix::SOURCES += qtservice_unix.cpp \
		qtunixserversocket.cpp \
		qtunixsocket.cpp

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
    opennicresolver.h

RESOURCES += \
		opennicserver.qrc

unix:LIBS += ../common/libcommon.a
win32:LIBS += ../common/debug/libcommon.a
win32:LIBS += C:/QtSDK/mingw/lib/libwsock32.a
