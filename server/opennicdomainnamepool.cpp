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
#include "opennicdomainnamepool.h"

#define inherited QObject

OpenNICDomainNamePool::OpenNICDomainNamePool(QObject *parent)
: inherited(parent)
{
}

OpenNICDomainNamePool::OpenNICDomainNamePool(const OpenNICDomainNamePool& other)
: inherited(NULL)
{
	copy(other);
}


OpenNICDomainNamePool::~OpenNICDomainNamePool()
{
}

/**
  * @brief copy from another
  */
OpenNICDomainNamePool& OpenNICDomainNamePool::copy(const OpenNICDomainNamePool& other)
{
	if ( &other != this )
	{
		mDomains = other.mDomains;
	}
	return *this;
}

/**
  * @brief copy from another
  */
OpenNICDomainNamePool& OpenNICDomainNamePool::operator=(const OpenNICDomainNamePool& other)
{
	return copy(other);
}

/**
  * @append a domain if it does not exists
  */
void OpenNICDomainNamePool::append(OpenNICDomainName& domain)
{
	if (indexOf(domain)<0)
	{
		mDomains.append(domain);
	}
}

/**
  * @brief get the index of a domain
  * @brief return the index or -1
  */
int OpenNICDomainNamePool::indexOf(OpenNICDomainName &domain)
{
	for(int n=0; n < mDomains.count(); n++)
	{
		OpenNICDomainName item = mDomains.at(n);
		if (item == domain)
		{
			return n;
		}
	}
	return -1;
}

/**
  * @return a string list
  */
QStringList OpenNICDomainNamePool::toStringList()
{
	QStringList rc;
	for(int n=0; n < mDomains.count(); n++)
	{
		OpenNICDomainName domain = mDomains.at(n);
		rc.append(domain.toString());
	}
	return rc;
}


