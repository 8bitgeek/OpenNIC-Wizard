/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
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
