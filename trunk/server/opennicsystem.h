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

#include "opennicdomainname.h"
#include "opennicdomainnamepool.h"

class OpenNICSystem
{
	public:
		static int						random(int low, int high) {return (qrand()%((high+1)-low)+low);}
		static bool						backup(QString filename);
		static bool						writeStringListToFile(QString filename,QStringList list);
		static bool						fileCopy(QString from, QString to);
		static bool						saveBootstrapT1List(QStringList tiList);
		static QStringList				getBootstrapT1List();
		static QStringList				getBootstrapT2List();
		static bool						saveTestDomains(QStringList domains);
		static OpenNICDomainNamePool	getTestDomains();
		static OpenNICDomainName		randomDomain();
		static QString					getSystemResolverList();

		static bool						beginUpdateResolvers(QString& output);
		static int						updateResolver(QHostAddress& dns,int index, QString& output);
		static bool						endUpdateResolvers(QString& output);
	private:
		static OpenNICDomainNamePool	mTestDomains;
};

#endif // OPENNICSYSTEM_H
