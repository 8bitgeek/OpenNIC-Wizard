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
#include "opennicdnsquery.h"
#include "opennicdnsquerylistener.h"

#include <QHostAddress>

#if defined(Q_OS_LINUX)
	#include <arpa/inet.h>
#else
    #include <stdint.h>
	// #include <winsock.h>
#endif

quint16					OpenNICDnsQuery::mMasterTid=0;
int						OpenNICDnsQuery::mQueries=0;

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
} query_header;

#define inherited QObject

OpenNICDnsQuery::OpenNICDnsQuery(QObject *parent)
: inherited(parent)
, mListener(NULL)
, mError(DNS_OK)
, mQueryType(DNS_A_RECORD)
, mTid(0)
, mPort(DEFAULT_DNS_PORT)
, mExpiryTimer(-1)
{
	++mQueries;
	mUDPSocket.bind();
	QObject::connect(&mUDPSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
	setStartTime(QDateTime::currentDateTime());
}

OpenNICDnsQuery::OpenNICDnsQuery(OpenNICDnsQueryListener* listener, QHostAddress resolver, OpenNICDomainName name, quint32 port, DNSQueryType queryType, QObject *parent)
: inherited(parent)
, mListener(listener)
, mResolver(resolver)
, mError(DNS_OK)
, mQueryType(queryType)
, mTid(0)
, mName(name)
, mPort(port)
, mExpiryTimer(-1)
{
	++mQueries;
	setListener(listener);
	mUDPSocket.bind();
	QObject::connect(&mUDPSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
	setStartTime(QDateTime::currentDateTime());
	lookup();
}

OpenNICDnsQuery::OpenNICDnsQuery(OpenNICDnsQueryListener* listener, QHostAddress resolver, OpenNICDomainName name, QDateTime expiryTime, quint32 port, DNSQueryType queryType, QObject *parent)
: inherited(parent)
, mListener(NULL)
, mResolver(resolver)
, mError(DNS_OK)
, mQueryType(queryType)
, mTid(0)
, mName(name)
, mPort(port)
, mExpiryTimer(-1)
{
	++mQueries;
	setListener(listener);
	mUDPSocket.bind();
	QObject::connect(&mUDPSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
	setStartTime(QDateTime::currentDateTime());
	setExpireTime(expiryTime);
	lookup();
}

OpenNICDnsQuery::OpenNICDnsQuery(OpenNICDnsQueryListener* listener, const OpenNICDnsQuery& other)
: inherited(NULL)
, mListener(NULL)
{
	++mQueries;
	copy(other);
	setListener(listener);
	mUDPSocket.bind();
	QObject::connect(&mUDPSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

}

OpenNICDnsQuery::OpenNICDnsQuery(const OpenNICDnsQuery& other)
: inherited(NULL)
, mListener(NULL)
{
	++mQueries;
	copy(other);
	mUDPSocket.bind();
	QObject::connect(&mUDPSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

}

OpenNICDnsQuery::~OpenNICDnsQuery()
{
	--mQueries;
	if (mUDPSocket.state() == QAbstractSocket::BoundState )
	{
		mUDPSocket.close();
	}
	//fprintf(stderr,"q=%d\n",mQueries);
}

/**
  * @brief copy from the other
  */
OpenNICDnsQuery& OpenNICDnsQuery::operator=(const OpenNICDnsQuery& other)
{
	return copy(other);
}

/**
  * @brief copy from the other
  */
OpenNICDnsQuery& OpenNICDnsQuery::copy(const OpenNICDnsQuery& other)
{
	if ( &other != this )
	{
		fprintf(stderr,"copy %d\n",mQueries);
		mError			= other.mError;
		mResolver		= other.mResolver;
		mQueryType		= other.mQueryType;
		mTid			= other.mTid;
		mStartTime		= other.mStartTime;
		mEndTime		= other.mEndTime;
		mExpireTime		= other.mExpireTime;
		mName			= other.mName;
		mAddr			= other.mAddr;
		mMxName			= other.mMxName;
		mPort			= other.mPort;
		if ( other.mExpiryTimer >= 0 )
		{
			setExpireTime(mExpireTime);
		}
	}
	return *this;
}

/**
  * @brief convert to string <resolver>;<domain>;<nic>;<latency>;<error>;<type>;<start>;<end>;
  */
QString OpenNICDnsQuery::toString()
{
	QString rc;
	rc += resolver().toString()			+ ";";
	rc += name().domainName()			+ ";";
	rc += name().dnsService()			+ ";";
	rc += QString::number(latency())	+ ";";
	rc += errorText()					+ ";";
	rc += queryTypeText()				+ ";";
	rc += startTime().toString("yyMMdd-hh:mm:ss.zzz")	+ ";";
	rc += endTime().toString("yyMMdd-hh:mm:ss.zzz")		+ ";";
	return rc;
}

/**
  * @brief query text
  */
QString OpenNICDnsQuery::queryTypeText()
{
	QString rc;
	switch(queryType())
	{
	case DNS_A_RECORD:			/* Lookup IP adress for host */
		rc = "DNS_A_RECORD";
		break;
	case DNS_MX_RECORD:			/* Lookup MX for domain */
		rc = "DNS_MX_RECORD";
		break;
	}
	return rc;
}

/**
  * @brief get the error string
  */
QString OpenNICDnsQuery::errorText()
{
	QString rc;
	switch(error())
	{
	case DNS_OK:				/* No error */
		rc = "DNS_OK";
		break;
	case DNS_DOES_NOT_EXIST:	/* Error: adress does not exist */
		rc = "DNS_DOES_NOT_EXIST";
		break;
	case DNS_TIMEOUT:			/* Lookup time expired */
		rc = "DNS_TIMEOUT";
		break;
	default:
	case DNS_ERROR:				/* No memory or other error */
		rc = "DNS_ERROR";
		break;
	}
	return rc;
}

/**
  * @brief set the listener
  */
void OpenNICDnsQuery::setListener(OpenNICDnsQueryListener *listener)
{
	if (mListener != NULL)
	{
		QObject::disconnect(this,SIGNAL(expired(OpenNICDnsQuery*)),mListener,SLOT(expired(OpenNICDnsQuery*)));
		QObject::disconnect(this,SIGNAL(finished(OpenNICDnsQuery*)),mListener,SLOT(finished(OpenNICDnsQuery*)));
		QObject::disconnect(this,SIGNAL(starting(OpenNICDnsQuery*)),mListener,SLOT(starting(OpenNICDnsQuery*)));
	}
	mListener = listener;
	if (mListener != NULL)
	{
		QObject::connect(this,SIGNAL(expired(OpenNICDnsQuery*)),mListener,SLOT(expired(OpenNICDnsQuery*)));
		QObject::connect(this,SIGNAL(finished(OpenNICDnsQuery*)),mListener,SLOT(finished(OpenNICDnsQuery*)));
		QObject::connect(this,SIGNAL(starting(OpenNICDnsQuery*)),mListener,SLOT(starting(OpenNICDnsQuery*)));
	}
}

/**
  * @brief calculate the latency
  */
quint64 OpenNICDnsQuery::latency()
{
	if (mEndTime>mStartTime)
	{
		return mStartTime.msecsTo(mEndTime);
	}
	return 0L;
}

/**
  * @brief set the start time end emit the start singal
  */
void OpenNICDnsQuery::setStartTime(QDateTime startTime)
{
	mStartTime = startTime;
	emit starting(this);
}

/**
  * @brief set the end time and emit the finished signal
  */
void OpenNICDnsQuery::setEndTime(QDateTime endTime)
{
	mEndTime = endTime;
}

/**
  * @brief set the expiry time
  */
void OpenNICDnsQuery::setExpireTime(QDateTime expireTime)
{
	QDateTime now = QDateTime::currentDateTime();

	if (mExpiryTimer >= 0 )
	{
		killTimer(mExpiryTimer);
		mExpiryTimer=-1;
	}
	mExpireTime = expireTime;
	if (mExpireTime > now)
	{
		mExpiryTimer = startTimer(now.msecsTo(mExpireTime));
	}
	else
	{
		emit expired(this);
		terminate();
	}
}

/**
  * @briref terminate the query. get here when reply is recieved, timeout, etc... (initiates end of query)
  */
void OpenNICDnsQuery::terminate()
{
	setEndTime(QDateTime::currentDateTime());
	if (mUDPSocket.state() == QAbstractSocket::BoundState )
	{
		mUDPSocket.close();
	}
	if (mExpiryTimer>=0)
	{
		killTimer(mExpiryTimer);
		mExpiryTimer=-1;
	}
	emit finished(this);
}

/**
  * @briref terminate the query. get here when reply is recieved, timeout, etc... (initiates end of query)
  */
void OpenNICDnsQuery::terminate(DNSError error)
{
	setError(error);
	terminate();
}

/**
  * @brief timer events
  */
void OpenNICDnsQuery::timerEvent(QTimerEvent *e)
{
	inherited::timerEvent(e);
	if (e->timerId() == mExpiryTimer)
	{
		emit expired(this);
		terminate();
	}
}

/*
 * Fetch name from DNS packet
 */
void OpenNICDnsQuery::fetch(const quint8 *pkt, const quint8 *s, int pktsiz, char *dst, int dstlen)
{
	const quint8 *e = pkt + pktsiz;
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
void OpenNICDnsQuery::processDatagram(QByteArray& datagram)
{
	quint8*				pkt = (quint8*)datagram.data();
	int					len = datagram.length();
	query_header*		pkt_hdr;
	const quint8		*p, *e;
	quint16				type;
	char				name[1025];
	int					found, stop, dlen, nlen;

	/* We sent 1 query. We want to see more that 1 answer. */
	pkt_hdr = (query_header *) pkt;
	if (ntohs(pkt_hdr->nqueries) != 1)
		return;

	/* Received 0 answers */
	if (pkt_hdr->nanswers == 0) {
		addr().clear();
		terminate(DNS_DOES_NOT_EXIST);
		return;
	}

	/* Skip host name */
	for (e = pkt + len, nlen = 0, p = &pkt_hdr->data[0];  p < e && *p != '\0';  p++)
		nlen++;

#define	NTOHS(p)	(((p)[0] << 8) | (p)[1])

	/* We sent query class 1, query type 1 */
	if (&p[5] > e || NTOHS(p + 1) != queryType())
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
		} else if (type == queryType()) {
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
			if (queryType() == DNS_MX_RECORD)
			{
				fetch((quint8*)pkt_hdr, p + 2, len, name, sizeof(name) - 1);
				p = (const unsigned char *)name;
				dlen = strlen(name);
				QByteArray mx((const char*)p,dlen);
				mxName().append(mx);
				terminate(DNS_OK);
			}
			else if (queryType() == DNS_A_RECORD)
			{
				QByteArray addr((const char*)p,dlen);
				if (dlen >= 4)
				{
					QString sAddr;
					sAddr = QString::number((quint8)addr[0])+"."+
							QString::number((quint8)addr[1])+"."+
							QString::number((quint8)addr[2])+"."+
							QString::number((quint8)addr[3]);
					this->addr().setAddress(sAddr);
					terminate(DNS_OK);
				}
			}
		}
	}

	//terminate(DNS_ERROR);
}

/**
  * @brief Read out all pending datagrams call processDatagram() for each one.
  */
void OpenNICDnsQuery::readPendingDatagrams()
{
	while (mUDPSocket.state() == QAbstractSocket::BoundState && mUDPSocket.hasPendingDatagrams())
	{
		QByteArray datagram;
		datagram.resize(mUDPSocket.pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		mUDPSocket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
		//fprintf(stderr,"receiving reply\n");
		processDatagram(datagram);
	}
}

/**
  * @brief Perform a DNS lookuop at the resolver address set by setResolver().
  * @return Results are emitted by the reply() signal.
  */
void OpenNICDnsQuery::lookup()
{
	query_header*	pkt_hdr;
	int				i, n, name_len;
	char			pkt[DNS_PACKET_LEN], *p;
	const char*		s;

	/* Prepare DNS packet header */
	pkt_hdr				= (query_header*)pkt;
	pkt_hdr->tid		= nextTid();
	pkt_hdr->flags		= htons(0x100);		/* Haha. guess what it is */
	pkt_hdr->nqueries	= htons(1);			/* Just one query */
	pkt_hdr->nanswers	= 0;
	pkt_hdr->nauth		= 0;
	pkt_hdr->nother		= 0;

	/* Encode DNS name */

	char sname[2048];
    strcpy(sname,name().domainName().toLocal8Bit().data());
	char* pname = &sname[0];
	name_len = strlen(pname);
	p = (char *) &pkt_hdr->data;	/* For encoding host name into packet */

	do {
		if ((s = strchr(pname, '.')) == NULL)
			s = pname + name_len;

        n = s - pname;              /* Chunk length */
        *p++ = n;                   /* Copy length */
		for (i = 0; i < n; i++)		/* Copy chunk */
			*p++ = pname[i];

		if (*s == '.')
			n++;

		pname += n;
		name_len -= n;

	} while (*s != '\0');

	*p++ = 0;						/* Mark end of host name */
	*p++ = 0;						/* Well, lets put this byte as well */
	*p++ = (quint8) queryType();	/* Query Type */

	*p++ = 0;
	*p++ = 1;						/* Class: inet, 0x0001 */

	if ( p < pkt + sizeof(pkt) )
	{
		n = p - pkt;					/* Total packet length */

		QByteArray datagram(pkt,n);
		//fprintf(stderr,"sending query packet\n");
		if ( mUDPSocket.writeDatagram(datagram,resolver(),port()) < 0 )
		{
			terminate(DNS_ERROR);
		}
	}
	else
	{
		terminate(DNS_ERROR);
	}
}




