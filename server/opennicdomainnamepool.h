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
#ifndef OPENNICDOMAINNAMEPOOL_H
#define OPENNICDOMAINNAMEPOOL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>

#include "opennicdomainname.h"

class OpenNICDomainNamePool : public QObject
{
    Q_OBJECT
	public:
		OpenNICDomainNamePool(QObject *parent = 0);
		OpenNICDomainNamePool(const OpenNICDomainNamePool& other);
		virtual ~OpenNICDomainNamePool();
		OpenNICDomainNamePool&		copy(const OpenNICDomainNamePool& other);
		OpenNICDomainNamePool&		operator=(const OpenNICDomainNamePool& other);
		void						append(OpenNICDomainName& domain);
		int							indexOf(OpenNICDomainName& domain);
		int							count()		{return mDomains.count();}
		OpenNICDomainName			at(int n)	{return mDomains.at(n);}
		QList<OpenNICDomainName>&	domains()	{return mDomains;}
		QStringList					toStringList();
	private:
		QList<OpenNICDomainName>	mDomains;
};

#endif // OPENNICDOMAINNAMEPOOL_H
