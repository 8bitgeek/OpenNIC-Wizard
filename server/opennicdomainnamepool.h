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
