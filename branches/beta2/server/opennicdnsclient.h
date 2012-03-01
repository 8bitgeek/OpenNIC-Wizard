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

		typedef enum {
			DNS_A_RECORD = 0x01,		/* Lookup IP adress for host */
			DNS_MX_RECORD = 0x0f		/* Lookup MX for domain */
		} dns_query_type;

		/*
		 * User defined function that will be called when DNS reply arrives for
		 * requested hostname. "struct dns_cb_data" is passed to the user callback,
		 * which has an error indicator, resolved address, etc.
		 */

		typedef enum {
			DNS_OK,						/* No error */
			DNS_DOES_NOT_EXIST,			/* Error: adress does not exist */
			DNS_TIMEOUT,				/* Lookup time expired */
			DNS_ERROR					/* No memory or other error */
		} dns_error;

		OpenNICDnsClient(bool active=true, QObject *parent = 0);
		virtual ~OpenNICDnsClient();
		bool						isActive() {return mActive;}
		dns_query*					find(void* context);
	public slots:
		virtual void				setActive(bool active) {mActive=active;}

	protected:
		virtual void				purge();
		void						setResolver(QHostAddress& resolverAddress);
		QHostAddress&				resolverAddress();
		bool						isOpen() {return mClientSocket != NULL; }
		bool						open();
		void						close();
		virtual void				lookup(QHostAddress resolverAddress, QString name, dns_query_type qtype, quint16 port=DEFAULT_DNS_PORT);
		virtual void				lookup(QString name, dns_query_type qtype, quint16 port=DEFAULT_DNS_PORT);
		virtual void				reply(dns_query& data);

	private slots:
		void						readPendingDatagrams();

	private:
		void						appendActiveQuery(dns_query* q);
		dns_query*					findActiveQuery(quint16 tid);
		void						disposeQuery(dns_query* q);
		void						fetch(const quint8 *pkt, const quint8 *s, int pktsiz, char *dst, int dstlen);
		void						doReply(dns_query* q, dns_error error);
		void						processDatagram(QByteArray datagram);
		bool						mActive;
		quint16						m_tid;				/* Latest tid used		*/
		QHostAddress				mResolverAddress;	/* The resolver address */
		QUdpSocket*					mClientSocket;		/* UDP socket used for queries	*/
		QList<dns_query*>			mQueries;			/* In-flight queries */
};

/*
 * User query. Holds mapping from application-level ID to DNS transaction id,
 * and user defined callback function.
 */
class dns_query {
public:
	dns_query()
		: latency(NULL)
		, error(OpenNICDnsClient::DNS_OK)
		, query_type(OpenNICDnsClient::DNS_A_RECORD)
		, tid(0)
	{}
	~dns_query() {}
	quint64								latency;		/* latency in milliseconds */
	OpenNICDnsClient::dns_error			error;			/* Result code */
	OpenNICDnsClient::dns_query_type	query_type;		/* Query type */
	quint16								tid;			/* UDP DNS transaction ID	*/
	QDateTime							start;			/* The start of the query */
	QDateTime							expire;			/* Time when this query expire	*/
	QString								name;			/* Host name			*/
	QHostAddress						addr;			/* Host address */
	QString								mxName;			/* MX record host name. */
};


#endif // OPENNICDNSCLIENT_H
