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

QT       += core gui network widgets

TARGET = OpenNIC
TEMPLATE = app
CONFIG += warn_on \
	thread \
	qt \
        uitools \
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
win32:LIBS += C:/QtSDK/mingw/lib/libwsock32.a
