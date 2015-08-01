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
#include "opennicdomainname.h"

#include <QString>
#include <QStringList>

#define ANNONYMOUS_SERVICE	"{annon}"

#define inherited QObject

OpenNICDomainName::OpenNICDomainName(QObject *parent)
: inherited(parent)
, mDnsService(ANNONYMOUS_SERVICE)
{
}

OpenNICDomainName::OpenNICDomainName(QString text, QObject *parent)
: inherited(parent)
{
	fromString(text);
}

OpenNICDomainName::OpenNICDomainName(const OpenNICDomainName& other)
: inherited(NULL)
{
	copy(other);
}

OpenNICDomainName::~OpenNICDomainName()
{
}

/**
  * @brief assignment operator
  */
OpenNICDomainName& OpenNICDomainName::operator=(const OpenNICDomainName& other)
{
	return copy(other);
}

/**
  * @brief equality operator
  */
bool OpenNICDomainName::operator==(OpenNICDomainName &other)
{
	return toString() == other.toString();
}

/**
  * @brief unequality operator
  */
bool OpenNICDomainName::operator!=(OpenNICDomainName &other)
{
	return toString() != other.toString();
}

/**
  * @brief Copy from another.
  */
OpenNICDomainName&	OpenNICDomainName::copy(const OpenNICDomainName& other)
{
	if ( &other != this )
	{
		mDnsService = other.mDnsService;
		mDomainName = other.mDomainName;
	}
	return *this;
}

/**
  * @brief convert from text.
  */
void OpenNICDomainName::fromString(QString text)
{
	QStringList parts = text.split(";");
	if (parts.count()>=2)
	{
		mDnsService = parts[0].trimmed();
		mDomainName = parts[1].trimmed();
	}
	else
	{
		mDnsService = ANNONYMOUS_SERVICE;
		mDomainName = text;
	}
}

/**
  * @brief convert to text
  */
QString OpenNICDomainName::toString()
{
	return mDnsService.trimmed()+";"+mDomainName.trimmed();
}

