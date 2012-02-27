/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICSYSTEM_H
#define OPENNICSYSTEM_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QMultiMap>
#include <QTimerEvent>
#include <QHostAddress>

class OpenNICSystem
{
	public:

		static QStringList			getBootstrapT1List();
		static QStringList			getBootstrapT2List();
		static QStringList			getTestDomains();
		static QString				getSystemResolverList();
		static QString				insertSystemResolver(QHostAddress dns,int index);
};

#endif // OPENNICSYSTEM_H
