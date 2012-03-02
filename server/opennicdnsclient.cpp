/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicdnsclient.h"

#if defined(Q_OS_WIN32) || defined(Q_WS_WIN32)
	#include <winsock.h>
#elif defined(Q_OS_LINUX)
	#include <arpa/inet.h>
#else
	#error "Platform not defined."
#endif

OpenNICDnsClient::OpenNICDnsClient(bool active, QObject *parent)
: QObject(parent)
, mActive(active)
, m_tid(0)
, mClientSocket(NULL)
{
}

/**
  * @brief Close a DNS client.
  */
OpenNICDnsClient::~OpenNICDnsClient()
{
	close();
	for(int n=0; n < mQueries.count(); n++)
	{
		dns_query* q = mQueries.takeAt(0);
		delete q;
	}
	mQueries.clear();
}

/**
  * @brief assemble a reply packet and emit it through the reply() sinal
  */
void OpenNICDnsClient::doReply(dns_query* q, dns_error error)
{
	q->error = error;
	reply(*q);
}


void OpenNICDnsClient::close()
{
	if (mClientSocket !=NULL )
	{
		mClientSocket->close();
		mClientSocket->deleteLater();
		mClientSocket = NULL;
	}
}

/**
  * @brief Open a UDP socket.
  */
bool OpenNICDnsClient::open()
{
	close();
	mClientSocket = new QUdpSocket();
	mClientSocket->bind();
	QObject::connect(mClientSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
	return true;
}

void OpenNICDnsClient::setResolver(QHostAddress &resolverAddress)
{
	mResolverAddress=resolverAddress;
}

QHostAddress& OpenNICDnsClient::resolverAddress()
{
	return mResolverAddress;
}

/**
  * @brief Read out all pending datagrams call processDatagram() for each one.
  */
void OpenNICDnsClient::readPendingDatagrams()
{
	while (mClientSocket->hasPendingDatagrams())
	{
		QByteArray datagram;
		datagram.resize(mClientSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		mClientSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
		processDatagram(datagram);
	}
}

/**
  * @brief get here once in a while to purge expired queries from the queue.
  */
void OpenNICDnsClient::purge()
{
	QDateTime now = QDateTime::currentDateTime();
	for(int n=0; n < mQueries.count(); n++ )
	{
		dns_query* q = mQueries.at(n);
		if ( q->expire < now )
		{
			q->addr.clear();
			doReply(q,DNS_TIMEOUT);
			disposeQuery(q);
		}
	}
}

/**
  * @brief Append an active query to the list of active queries.
  */
void OpenNICDnsClient::appendActiveQuery(dns_query* q)
{
	mQueries.append(q);
}

/**
  * @brief Dispose a query.
  */
void OpenNICDnsClient::disposeQuery(dns_query *q)
{
	int n = mQueries.indexOf(q);
	if ( n >= 0 )
	{
		delete mQueries.takeAt(n);
	}
}

/**
  * @brief Match a tid to a pending query.
  * @return the query or NULL.
  */
dns_query* OpenNICDnsClient::findActiveQuery(quint16 tid)
{
	for(int n=0; n < mQueries.count(); n++)
	{
		dns_query* q = mQueries.at(n);
		if (tid == q->tid)
		{
			return q;
		}
	}
	return NULL;
}

/*
 * Fetch name from DNS packet
 */
void OpenNICDnsClient::fetch(const quint8 *pkt, const quint8 *s, int pktsiz, char *dst, int dstlen)
{
	const uint8_t	*e = pkt + pktsiz;
	int		j, i = 0, n = 0;

	while (*s != 0 && s < e)
	{
		if (n > 0)
			dst[i++] = '.';

		if (i >= dstlen)
			break;

		if ((n = *s++) == 0xc0)
		{
			s = pkt + *s;	/* New offset */
			n = 0;
		}
		else
		{
			for (j = 0; j < n && i < dstlen; j++)
				dst[i++] = *s++;
		}
	}

	dst[i] = '\0';
}

/**
  * @brief Process the datagram
  * @param datagram The raw datagram octets
  */
void OpenNICDnsClient::processDatagram(QByteArray& datagram)
{
	quint8*			pkt = (quint8*)datagram.data();
	int				len = datagram.length();
	header*			pkt_hdr;
	const quint8	*p, *e, *s;
	dns_query*		q;
	quint16			type;
	char			name[1025];
	int				found, stop, dlen, nlen;

	/* We sent 1 query. We want to see more that 1 answer. */
	pkt_hdr = (header *) pkt;
	if (ntohs(pkt_hdr->nqueries) != 1)
		return;

	/* Return if we did not send that query */
	if ((q = findActiveQuery(pkt_hdr->tid)) == NULL)
		return;

	/* Received 0 answers */
	if (pkt_hdr->nanswers == 0) {
		q->addr.clear();
		doReply(q, DNS_DOES_NOT_EXIST);
		return;
	}

	/* Skip host name */
	for (e = pkt + len, nlen = 0, s = p = &pkt_hdr->data[0];  p < e && *p != '\0';  p++)
		nlen++;

#define	NTOHS(p)	(((p)[0] << 8) | (p)[1])

	/* We sent query class 1, query type 1 */
	if (&p[5] > e || NTOHS(p + 1) != q->query_type)
		return;

	/* Go to the first answer section */
	p += 5;

	/* Loop through the answers, we want A type answer */
	for (found = stop = 0; !stop && &p[12] < e; ) {

		/* Skip possible name in CNAME answer */
		if (*p != 0xc0) {
			while (*p && &p[12] < e)
				p++;
			p--;
		}

		type = htons(((uint16_t *)p)[1]);

		if (type == 5) {
			/* CNAME answer. shift to the next section */
			dlen = htons(((uint16_t *) p)[5]);
			p += 12 + dlen;
		} else if (type == q->query_type) {
			found = stop = 1;
		} else {
			stop = 1;
		}
	}

	if (found && &p[12] < e) {
		dlen = htons(((uint16_t *) p)[5]);
		p += 12;

		if (p + dlen <= e) {

			/* Call user */
			if (q->query_type == DNS_MX_RECORD)
			{
				fetch((quint8*)pkt_hdr, p + 2, len, name, sizeof(name) - 1);
				p = (const unsigned char *)name;
				dlen = strlen(name);
				QByteArray mx((const char*)p,dlen);
				q->mxName.append(mx);
				doReply(q, DNS_OK);
				disposeQuery(q);
			}
			else if (q->query_type == DNS_A_RECORD)
			{
				QByteArray addr((const char*)p,dlen);
				if (dlen >= 4)
				{
					QString sAddr;
					sAddr = QString::number((quint8)addr[0])+"."+
							QString::number((quint8)addr[1])+"."+
							QString::number((quint8)addr[2])+"."+
							QString::number((quint8)addr[3]);
					q->addr.setAddress(sAddr);
					doReply(q, DNS_OK);
					disposeQuery(q);
				}
				else
				{
					doReply(q,DNS_ERROR);
					disposeQuery(q);
				}
			}
			else
			{
				doReply(q,DNS_ERROR);
				disposeQuery(q);
			}
		}
	}
}

/**
  * @brief Perform a DNS lookup at the given resolver address.
  * @param resolverAddress The IP address of the resolver
  * @param name The name to lookup
  * @param type the query type.
  * @param port the port address at the resolver
  * @return Resultes are emitted by the reply() signal.
  */
void OpenNICDnsClient::lookup(QHostAddress resolverAddress, OpenNICDomainName name, dns_query_type type, quint16 port)
{
	setResolver(resolverAddress);
	lookup(name,type,port);
}

/**
  * @brief Perform a DNS lookuop at the resolver address set by setResolver().
  * @param name The name to lookup
  * @param type the query type.
  * @param port the port address at the resolver
  * @return Results are emitted by the reply() signal.
  */
void OpenNICDnsClient::lookup(OpenNICDomainName name, dns_query_type qtype, quint16 port)
{
	dns_query* q;

	if ( !isOpen() )
	{
		open();
	}
	if ( isOpen() && (q = new dns_query) != NULL )
	{
		QDateTime now = QDateTime::currentDateTime();

		header*		pkt_hdr;
		int			i, n, name_len;
		char		pkt[DNS_PACKET_LEN], *p;
		const char*	s;

		/* Init query structure */
		q->query_type	= qtype;
		q->tid		= ++m_tid;
		q->expire	= now.addSecs(DNS_QUERY_TIMEOUT);
		q->name		= name;

		/* Prepare DNS packet header */
		pkt_hdr				= (header*)pkt;
		pkt_hdr->tid		= q->tid;
		pkt_hdr->flags		= htons(0x100);		/* Haha. guess what it is */
		pkt_hdr->nqueries	= htons(1);			/* Just one query */
		pkt_hdr->nanswers	= 0;
		pkt_hdr->nauth		= 0;
		pkt_hdr->nother		= 0;

		/* Encode DNS name */

		char sname[2048];
		strcpy(sname,name.domainName().toAscii().data());
		char* pname = &sname[0];
		name_len = strlen(pname);
		p = (char *) &pkt_hdr->data;	/* For encoding host name into packet */

		do {
			if ((s = strchr(pname, '.')) == NULL)
				s = pname + name_len;

			n = s - pname;			/* Chunk length */
			*p++ = n;			/* Copy length */
			for (i = 0; i < n; i++)		/* Copy chunk */
				*p++ = pname[i];

			if (*s == '.')
				n++;

			pname += n;
			name_len -= n;

		} while (*s != '\0');

		*p++ = 0;						/* Mark end of host name */
		*p++ = 0;						/* Well, lets put this byte as well */
		*p++ = (quint8) qtype;			/* Query Type */

		*p++ = 0;
		*p++ = 1;						/* Class: inet, 0x0001 */

		if ( !(p < pkt + sizeof(pkt)) )
		{
			doReply(q,DNS_ERROR);
			return;
		}
		n = p - pkt;					/* Total packet length */

		QByteArray datagram(pkt,n);
		if ( mClientSocket->writeDatagram(datagram,resolverAddress(),port) < 0 )
		{
			doReply(q,DNS_ERROR);
			delete q;
		}
		appendActiveQuery(q);
	}
	else
	{
		doReply(q,DNS_ERROR);
		delete q;
		return;
	}
}


