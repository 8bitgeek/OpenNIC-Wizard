/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICDNSQUERY_H
#define OPENNICDNSQUERY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QHostAddress>

#include "opennicdomainname.h"

/*
 * User query. Holds mapping from application-level ID to DNS transaction id,
 * and user defined callback function.
 */
class OpenNICDnsQuery : public QObject
{
	Q_OBJECT
	public:
		typedef enum {
			DNS_A_RECORD = 0x01,			/* Lookup IP adress for host */
			DNS_MX_RECORD = 0x0f			/* Lookup MX for domain */
		} DNSQueryType;

		typedef enum {
			DNS_OK,							/* No error */
			DNS_DOES_NOT_EXIST,				/* Error: adress does not exist */
			DNS_TIMEOUT,					/* Lookup time expired */
			DNS_ERROR						/* No memory or other error */
		} DNSError;

		typedef enum
		{
			Red=0,							/* service is down */
			Yellow,							/* service may be down */
			Green							/* service is up */
		} ColourCode;

		OpenNICDnsQuery(QObject *parent = 0);
		OpenNICDnsQuery(const OpenNICDnsQuery& other);
		virtual ~OpenNICDnsQuery();
		OpenNICDnsQuery&					operator=(const OpenNICDnsQuery& other);
		OpenNICDnsQuery&					copy(const OpenNICDnsQuery& other);

		quint64								latency();
		DNSError							error()									{return mError;}
		DNSQueryType						queryType()								{return mQueryType;}
		quint16								tid()									{return mTid;}
		QDateTime&							startTime()								{return mStartTime;}
		QDateTime&							endTime()								{return mEndTime;}
		QDateTime&							expireTime()							{return mExpireTime;}
		OpenNICDomainName&					name()									{return mName;}
		QHostAddress&						addr()									{return mAddr;}
		QString&							mxName()								{return mMxName;}

	signals:
		void								finished(OpenNICDnsQuery* query);

	public slots:
		void								setError(DNSError& error)				{mError = error;}
		void								setQueryType(DNSQueryType& queryType)	{mQueryType = queryType;}
		void								setTid(quint16 tid)						{mTid = tid;}
		void								setStartTime(QDateTime startTime)		{mStartTime = startTime;}
		void								setEndTime(QDateTime endTime);
		void								setExpireTime(QDateTime expireTime)		{mExpireTime = expireTime;}
		void								setName(OpenNICDomainName& name)		{mName = name;}
		void								setAddr(QHostAddress& addr)				{mAddr = addr;}
		void								setMxName(QString& mxName)				{mMxName = mxName;}

	private:
		DNSError							mError;									/* Result code */
		DNSQueryType						mQueryType;								/* Query type */
		quint16								mTid;									/* UDP DNS transaction ID	*/
		QDateTime							mStartTime;								/* The start of the query */
		QDateTime							mEndTime;								/* The end of the query */
		QDateTime							mExpireTime;							/* Time when this query expire	*/
		OpenNICDomainName					mName;									/* Host name			*/
		QHostAddress						mAddr;									/* Host address */
		QString								mMxName;								/* MX record host name. */
};

#endif // OPENNICDNSQUERY_H





