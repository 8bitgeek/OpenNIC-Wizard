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
