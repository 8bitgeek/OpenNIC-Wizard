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
#ifndef OPENNICDOMAINNAME_H
#define OPENNICDOMAINNAME_H

#include <QObject>
#include <QString>

/**
  * @brief input text is <DNS Service>;<domain name> example: "opennic;www.opennic.glue"
  */
class OpenNICDomainName : public QObject
{
	Q_OBJECT
	public:
		OpenNICDomainName(QObject *parent = 0);
		OpenNICDomainName(QString text, QObject *parent = 0);
		OpenNICDomainName(const OpenNICDomainName& other);
		virtual ~OpenNICDomainName();

		OpenNICDomainName&	copy(const OpenNICDomainName& other);

		OpenNICDomainName&	operator=(const OpenNICDomainName& other);
		bool				operator==(OpenNICDomainName &other);
		bool				operator!=(OpenNICDomainName &other);

		void				fromString(QString text);
		QString				toString();

		void				setDnsService(QString dnsService)	{mDnsService=dnsService;}
		void				setDomainName(QString domainName)	{mDomainName=domainName;}

		QString&			dnsService()		{return mDnsService;}
		QString&			domainName()		{return mDomainName;}

	private:
		QString				mDnsService;
		QString				mDomainName;

};

#endif // OPENNICDOMAINNAME_H
