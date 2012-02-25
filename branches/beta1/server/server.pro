#-------------------------------------------------
#
# Project created by QtCreator 2012-02-21T22:22:50
#
#-------------------------------------------------

QT  += core network

TARGET = OpenNICServer
TEMPLATE = app
CONFIG += warn_on \
	thread \
	qt \
	uitools \
	debug

SOURCES += main.cpp\
		opennicserver.cpp \
		opennicresolver.cpp \
		opennictest.cpp \
		opennicdns.cpp \
		openniclog.cpp

HEADERS  += opennicserver.h \
		opennicresolver.h \
		opennictest.h \
		opennicdns.h \
		openniclog.h

RESOURCES += \
		opennicserver.qrc

win32:LIBS += C:/QtSDK/mingw/lib/libwsock32.a


