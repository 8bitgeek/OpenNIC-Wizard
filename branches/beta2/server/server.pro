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
		opennicdns.cpp \
		openniclog.cpp \
		qtservice.cpp \
    opennicresolverpoolitem.cpp \
    opennicresolverpool.cpp \
    opennicsystem.cpp \
    opennicresolvertest.cpp


win32::SOURCES += qtservice_win.cpp

unix::SOURCES += qtservice_unix.cpp \
		qtunixserversocket.cpp \
		qtunixsocket.cpp

HEADERS  += opennicserver.h \
		opennicdns.h \
		openniclog.h \
		qtservice.h \
		qtservice_p.h \
		qtunixserversocket.h \
		qtunixsocket.h \
    opennicresolverpoolitem.h \
    opennicresolverpool.h \
    opennicsystem.h \
    opennicresolvertest.h

RESOURCES += \
		opennicserver.qrc

win32:LIBS += C:/QtSDK/mingw/lib/libwsock32.a
