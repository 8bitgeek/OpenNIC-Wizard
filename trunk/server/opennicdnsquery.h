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
#include <QTimerEvent>
#include <QUdpSocket>
#include <QMultiHash>

#include "opennicdomainname.h"

#define	DNS_QUERY_TIMEOUT	30		/* Query timeout, seconds	*/
#define	DNS_MAX				1025	/* Maximum host name		*/
#define	DNS_PACKET_LEN		2048	/* Buffer size for DNS packet	*/
#define DEFAULT_DNS_PORT	53		/* The default DNS UDP oprt */

/*
 * User query. Holds mapping from application-level ID to DNS transaction id,
 * and user defined callback function.
 */
class OpenNICDnsQueryListener;
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
		OpenNICDnsQuery(OpenNICDnsQueryListener* listener, QHostAddress resolver, OpenNICDomainName name, quint32 port = DEFAULT_DNS_PORT, DNSQueryType queryType=DNS_A_RECORD, QObject *parent = 0);
		OpenNICDnsQuery(OpenNICDnsQueryListener* listener, QHostAddress resolver, OpenNICDomainName name, QDateTime expiryTime, quint32 port = DEFAULT_DNS_PORT, DNSQueryType queryType=DNS_A_RECORD, QObject *parent = 0);
		OpenNICDnsQuery(const OpenNICDnsQuery& other);
		virtual ~OpenNICDnsQuery();
		OpenNICDnsQuery&					operator=(const OpenNICDnsQuery& other);
		OpenNICDnsQuery&					copy(const OpenNICDnsQuery& other);
		OpenNICDnsQueryListener*			listener()								{return mListener;}
		quint64								latency();
		QHostAddress&						resolver()								{return mResolver;}
		DNSError							error()									{return mError;}
		DNSQueryType						queryType()								{return mQueryType;}
		QDateTime&							startTime()								{return mStartTime;}
		QDateTime&							endTime()								{return mEndTime;}
		QDateTime&							expireTime()							{return mExpireTime;}
		OpenNICDomainName&					name()									{return mName;}
		QHostAddress&						addr()									{return mAddr;}
		QString&							mxName()								{return mMxName;}
		quint32								port()									{return mPort;}
		static QMultiHash<OpenNICDnsQueryListener*,OpenNICDnsQuery*> queries()		{return mQueries;}
	signals:
		void								starting(OpenNICDnsQuery* query);
		void								finished(OpenNICDnsQuery* query);
		void								expired(OpenNICDnsQuery* query);
	protected:
		virtual void						timerEvent(QTimerEvent *);
	public slots:
		void								lookup();
		void								terminate();
		void								terminate(DNSError error);
		void								setResolver(QHostAddress& resolver)		{mResolver = resolver;}
		void								setError(DNSError& error)				{mError = error;}
		void								setQueryType(DNSQueryType& queryType)	{mQueryType = queryType;}
		void								setExpireTime(QDateTime expireTime);
		void								setName(OpenNICDomainName& name)		{mName = name;}
		void								setPort(quint32 port)					{mPort = port;}
	private slots:
		void								readPendingDatagrams();
	private:
		static quint16						nextTid()								{return ++mMasterTid;}
		quint16								tid()									{return mTid;}
		void								setTid(quint16 tid)						{mTid = tid;}
		void								fetch(const quint8 *pkt, const quint8 *s, int pktsiz, char *dst, int dstlen);
		void								processDatagram(QByteArray& datagram);
		void								setStartTime(QDateTime startTime);
		void								setEndTime(QDateTime endTime);
	private:
		static QMultiHash<OpenNICDnsQueryListener*,OpenNICDnsQuery*> mQueries;		/* all queries */
		static quint16						mMasterTid;
		OpenNICDnsQueryListener*			mListener;
		QHostAddress						mResolver;								/* Resolver Host address */
		DNSError							mError;									/* Result code */
		DNSQueryType						mQueryType;								/* Query type */
		quint16								mTid;									/* UDP DNS transaction ID	*/
		QDateTime							mStartTime;								/* The start of the query */
		QDateTime							mEndTime;								/* The end of the query */
		QDateTime							mExpireTime;							/* Time when this query expire	*/
		OpenNICDomainName					mName;									/* Host name			*/
		QHostAddress						mAddr;									/* Host address */
		QString								mMxName;								/* MX record host name. */
		quint32								mPort;									/* UDP port # */
		int									mExpiryTimer;							/* expiry timer */
		QUdpSocket							mUDPSocket;								/* UDP socket used for queries	*/
};

#endif // OPENNICDNSQUERY_H





