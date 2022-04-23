# This file is a part of OpenNIC Wizard
# Copywrong (c) 2012-2022 Mike Sharkey
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 1776):
# <mike@8bitgeek.net> wrote this file.
# As long as you retain this notice you can do whatever you want with this
# stuff. If we meet some day, and you think this stuff is worth it,
# you can buy me a beer in return. ~ Mike Sharkey
# ----------------------------------------------------------------------------

QT       += core network

TARGET = common
TEMPLATE = lib
CONFIG += staticlib \
	warn_on \
	thread \
	qt \
	debug

SOURCES += \ 
    opennicnet.cpp \
    opennicpacket.cpp \
    opennicsettings.cpp

HEADERS  += \ 
    opennicnet.h \
    opennicpacket.h \
    opennicversion.h \
    opennicsettings.h

FORMS    += 

RESOURCES += 
