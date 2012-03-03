#-------------------------------------------------
#
# Project created by QtCreator 2012-02-21T22:22:50
#
#-------------------------------------------------

QT += core gui network

TARGET = OpenNIC
TEMPLATE = subdirs
CONFIG += warn_on \
	thread \
	qt \
	uitools \
	debug

SUBDIRS = server \
		client
