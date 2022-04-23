/*
 * This file is a part of OpenNIC Wizard
 * Copywrong (c) 2012-2022 Mike Sharkey
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 1776):
 * <mike@8bitgeek.net> wrote this file.
 * As long as you retain this notice you can do whatever you want with this
 * stuff. If we meet some day, and you think this stuff is worth it,
 * you can buy me a beer in return. ~ Mike Sharkey
 * ----------------------------------------------------------------------------
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

