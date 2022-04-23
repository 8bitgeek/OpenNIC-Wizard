# This file is a part of OpenNIC Wizard
# Copywrong (c) 2012-2022 Mike Sharkey
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 1776):
# <mike@8bitgeek.net> wrote this file.
# As long as you retain this notice you can do whatever you want with this
# stuff. If we meet some day, and you think this stuff is worth it,
# you can buy me a beer in return. ~ Mike Sharkey
# ----------------------------------------------------------------------------

QT += core gui network widgets

TARGET = OpenNIC
TEMPLATE = subdirs
CONFIG += warn_on \
	thread \
	qt \
        debug

SUBDIRS = common \
		server \
		client

CFLAGS += -Wno-implicit-fallthrough
