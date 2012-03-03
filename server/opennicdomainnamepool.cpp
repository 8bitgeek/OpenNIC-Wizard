/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
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


