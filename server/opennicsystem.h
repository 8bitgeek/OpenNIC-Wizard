/*
 *   This file is a part of OpenNIC Wizard
 *   Copyright (C) 2012-2015  Mike Sharkey <mike@8bitgeek.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
