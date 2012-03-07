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

win32:LIBS += C:/QtSDK/mingw/lib/libwsock32.a
