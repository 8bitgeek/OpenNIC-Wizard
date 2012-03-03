/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICDNSCLIENT_H
#define OPENNICDNSCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTime>
#include <QDateTime>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QTimerEvent>

#include "opennicdomainname.h"
#include "opennicdnsquery.h"

#define	DNS_QUERY_TIMEOUT	30		/* Query timeout, seconds	*/
#define	DNS_MAX				1025	/* Maximum host name		*/
#define	DNS_PACKET_LEN		2048	/* Buffer size for DNS packet	*/
#define DEFAULT_DNS_PORT	53		/* The default DNS UDP oprt */

class dns_query;
class OpenNICDnsClient : public QObject
{
	Q_OBJECT
	public:

		/*
		 * DNS network packet
		 */
		typedef struct {
			quint16		tid;		/* Transaction ID		*/
			quint16		flags;		/* Flags			*/
			quint16		nqueries;	/* Questions			*/
			quint16		nanswers;	/* Answers			*/
			quint16		nauth;		/* Authority PRs		*/
			quint16		nother;		/* Other PRs			*/
			quint8		data[1];	/* Data, variable length	*/
		} header;

		/*
		 * User defined function that will be called when DNS reply arrives for
		 * requested hostname. "struct dns_cb_data" is passed to the user callback,
		 * which has an error indicator, resolved address, etc.
		 */

		OpenNICDnsClient(bool active=true, QObject *parent = 0);
		virtual ~OpenNICDnsClient();
		bool						isActive() {return mActive;}
		OpenNICDnsQuery*			find(void* context);
	public slots:
		virtual void				setActive(bool active) {mActive=active;}

	protected:
		virtual void				purge();
		void						setResolver(QHostAddress& resolverAddress);
		QHostAddress&				resolverAddress();
		bool						isOpen() {return mClientSocket != NULL; }
		bool						open();
		void						close();
		virtual void				lookup(QHostAddress resolverAddress, OpenNICDomainName name, OpenNICDnsQuery::DNSQueryType qtype, quint16 port=DEFAULT_DNS_PORT);
		virtual void				lookup(OpenNICDomainName name, OpenNICDnsQuery::DNSQueryType qtype, quint16 port=DEFAULT_DNS_PORT);
		virtual void				reply(OpenNICDnsQuery& /* data */) {}

	private slots:
		void						readPendingDatagrams();

	private:
		void						appendActiveQuery(OpenNICDnsQuery* q);
		OpenNICDnsQuery*			findActiveQuery(quint16 tid);
		void						disposeQuery(OpenNICDnsQuery* q);
		void						fetch(const quint8 *pkt, const quint8 *s, int pktsiz, char *dst, int dstlen);
		void						doReply(OpenNICDnsQuery* q, OpenNICDnsQuery::DNSError error);
		void						processDatagram(QByteArray& datagram);
		bool						mActive;
		quint16						m_tid;				/* Latest tid used		*/
		QHostAddress				mResolverAddress;	/* The resolver address */
		QUdpSocket*					mClientSocket;		/* UDP socket used for queries	*/
		QList<OpenNICDnsQuery*>		mQueries;			/* In-flight queries */
};



#endif // OPENNICDNSCLIENT_H
