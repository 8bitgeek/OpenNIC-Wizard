/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicresolverpoolitem.h"
#include "opennicsystem.h"

#define		MAX_HISTORY_DEPTH			30				/* the default maximum history depth */
#define		BIG_LATENCY					100000			/* for uninitialized latencies so they come up at bottom fo a sort */
#define		RANDOM_INTERVAL_MIN			(10*1000)		/* milliseconds */
#define		RANDOM_INTERVAL_MAX			((10*60)*1000)	/* milliseconds */

#define inherited OpenNICDnsQueryListener

OpenNICResolverPoolItem::OpenNICResolverPoolItem(QObject* parent)
: inherited(parent)
, mScore(0.0)
, mMaxHistoryDepth(MAX_HISTORY_DEPTH)
, mQueryIntervalTimer(-1)
{
	clear();
}

OpenNICResolverPoolItem::OpenNICResolverPoolItem(QHostAddress hostAddress, QString kind, QObject* parent)
: inherited(parent)
, mScore(0.0)
, mMaxHistoryDepth(MAX_HISTORY_DEPTH)
, mQueryIntervalTimer(-1)
{
	clear();
	mHostAddress = hostAddress;
	mKind = kind;
	test();
}

OpenNICResolverPoolItem::OpenNICResolverPoolItem(const OpenNICResolverPoolItem& other)
: inherited(NULL)
, mScore(0.0)
, mMaxHistoryDepth(MAX_HISTORY_DEPTH)
, mQueryIntervalTimer(-1)
{
	copy(other);
	test();
}

/**
  * @brief Close a UDP socket.
  */
OpenNICResolverPoolItem::~OpenNICResolverPoolItem()
{
	clear();
}

/**
  * @brief equality operator
  */
bool OpenNICResolverPoolItem::operator==(OpenNICResolverPoolItem &other)
{
	return (hostAddress() == other.hostAddress());
}

/**
  * @brief unequality operator
  */
bool OpenNICResolverPoolItem::operator!=(OpenNICResolverPoolItem &other)
{
	return (hostAddress() != other.hostAddress());
}

/**
  * @brief greater-than operator (greater means less latency)
  */
bool OpenNICResolverPoolItem::operator>(OpenNICResolverPoolItem &other)
{
	return score() > other.score();
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolverPoolItem::operator<(OpenNICResolverPoolItem &other)
{
	return score() < other.score();
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolverPoolItem::operator>=(OpenNICResolverPoolItem &other)
{
	return score() >= other.score();
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolverPoolItem::operator<=(OpenNICResolverPoolItem &other)
{
	return score() <= other.score();
}


/**
  * @brief assignment operator.
  */
OpenNICResolverPoolItem& OpenNICResolverPoolItem::operator=(const OpenNICResolverPoolItem &other)
{
	return copy(other);
}

/**
  * @brief get the date/time of the last reply in the queue
  */
QDateTime OpenNICResolverPoolItem::lastReply()
{
	QDateTime rc;
	for(int n=0; n < mHistory.count(); n++)
	{
		OpenNICDnsQuery query = mHistory.at(n);
		if ( query.error() == OpenNICDnsQuery::DNS_DOES_NOT_EXIST || query.error() == OpenNICDnsQuery::DNS_OK )
		{
			rc = query.endTime();
			break;
		}
	}
	return rc;
}

/**
  * @brief get the date/time of the last timeout
  */
QDateTime OpenNICResolverPoolItem::lastTimeout()
{
	QDateTime rc;
	for(int n=0; n < mHistory.count(); n++)
	{
		OpenNICDnsQuery query = mHistory.at(n);
		if ( query.error() == OpenNICDnsQuery::DNS_TIMEOUT )
		{
			rc = query.endTime();
			break;
		}
	}
	return rc;
}

/**
  * @brief get the date/time of the last error
  */
QString OpenNICResolverPoolItem::lastFault()
{
	QString rc;
	if (mHistory.count())
	{
		OpenNICDnsQuery query = mHistory.at(0);
		if (query.error() == OpenNICDnsQuery::DNS_OK)
		{
			rc="DNS_OK " + query.addr().toString() + " " + query.name().domainName()+" ("+query.name().dnsService()+")";
		}
		else if (query.error() == OpenNICDnsQuery::DNS_DOES_NOT_EXIST)
		{
			rc="DNS_DOES_NOT_EXIST " + query.name().domainName()+" ("+query.name().dnsService()+")";
		}
		else if (query.error() == OpenNICDnsQuery::DNS_TIMEOUT)
		{
			rc="DNS_TIMEOUT " + query.name().domainName()+" ("+query.name().dnsService()+")";
		}
		else if (query.error() == OpenNICDnsQuery::DNS_ERROR)
		{
			rc="DNS_ERROR " + query.name().domainName()+" ("+query.name().dnsService()+")";
		}

	}
	return rc;
}

/**
  * @brief test of the resolver is alive
  */
bool OpenNICResolverPoolItem::alive()
{
	int deadCount=0;
	for(int n=0; n < mHistory.count(); n++)
	{
		OpenNICDnsQuery query = mHistory.at(n);
		if ( query.error() == OpenNICDnsQuery::DNS_TIMEOUT || query.error() == OpenNICDnsQuery::DNS_ERROR )
		{
			++deadCount;
		}
		else
		{
			break;
		}
	}
	return !(deadCount >= 2); /* two strikes it's out */
}

/**
  * @brief convert to a formatted string
  * @brief <hostAddress>;<avgLatency>;<testCount>;<replyCount>;<lastReply>;<lastTimeout>;<lastFault>;<kind>;
  */
QString& OpenNICResolverPoolItem::toString()
{
	mString.clear();
	mString += hostAddress().toString() + ";";
	mString += QString::number((int)averageLatency()) + ";";
	mString += QString::number(testCount()) + ";";
	mString += QString::number(replyCount()) + ";";
	mString += QString::number(timeoutCount()) + ";";
	mString += lastReply().toString() + ";";
	mString += lastTimeout().toString() + ";";
	mString += lastFault() + ";";
	mString += kind() + ";";
	return mString;
}

/**
  * @brief copy from another.
  * @return a self reference
  */
OpenNICResolverPoolItem& OpenNICResolverPoolItem::copy(const OpenNICResolverPoolItem& other)
{
	if ( &other != this )
	{
		mScore				= other.mScore;
		mHistory			= other.mHistory;
		mMaxHistoryDepth	= other.mMaxHistoryDepth;
		mHostAddress		= other.mHostAddress;
		mKind				= other.mKind;
	}
	return *this;
}

/**
  * @brief reset all internals to default state.
  */
void OpenNICResolverPoolItem::clear()
{
	mHistory.clear();
	mMaxHistoryDepth=MAX_HISTORY_DEPTH;
	mHostAddress.clear();
	mKind.clear();
}

/**
  * @brief set the maximum history depth.
  */
void OpenNICResolverPoolItem::setMaxHistoryDepth(int maxHistoryDepth)
{
	if (maxHistoryDepth>=1)
	{
		mMaxHistoryDepth = maxHistoryDepth;
	}
}

/**
  * @brief Return the number of tests which have been performed and are still on record in the history
  */
int OpenNICResolverPoolItem::testCount()
{
	return history().count();
}

/**
  * @brief replies are those dns queries that came back as either DNS_OK or DNS_DOES_NOT_EXIST meaning communication with the resolver was established.
  * @return the number of replies in the history for this resolver.
  */
int OpenNICResolverPoolItem::replyCount()
{
	int count=0;
	int nHistory = mHistory.count();
	for(int n=0; n < nHistory; n++)
	{
		OpenNICDnsQuery query = mHistory[n];
		if ( query.error() == OpenNICDnsQuery::DNS_OK || query.error() == OpenNICDnsQuery::DNS_DOES_NOT_EXIST )
		{
			++count;
		}
	}
	return count;
}

/**
  * @return the number of timeouts in the history buffer.
  */
int OpenNICResolverPoolItem::timeoutCount()
{
	int count=0;
	int nHistory = mHistory.count();
	for(int n=0; n < nHistory; n++)
	{
		OpenNICDnsQuery query = mHistory[n];
		if ( query.error() == OpenNICDnsQuery::DNS_TIMEOUT )
		{
			++count;
		}
	}
	return count;
}

/**
  * @return the latest latency.
  */
int OpenNICResolverPoolItem::lastLatency()
{
	int latency=BIG_LATENCY; /* something very big if there are no samples */
	if ( mHistory.count() > 0 )
	{
		latency = mHistory[0]->latency();
	}
	return latency;
}

/**
  * @return the average latency from the history buffer.
  */
double OpenNICResolverPoolItem::averageLatency()
{
	double total=0.0;
	int nHistory = mHistory.count();
	if ( nHistory > 0 )
	{
		for(int n=0; n < nHistory; n++)
		{
			total += mHistory[n]->latency();
		}
		return total/nHistory;
	}
	return BIG_LATENCY;
}

/**
  * @brief calculate the score
  */
double OpenNICResolverPoolItem::score()
{
	double rc=0.0;
	/** FIXME calculate the pool item scores */
	return rc;
}

/**
  * @brief prune history record back to the limit.
  */
void OpenNICResolverPoolItem::pruneHistory()
{
	/* prune... */
	while(mHistory.count()>0 && mHistory.count()>maxHistoryDepth())
	{
		mHistory.takeFirst();
	}
}

/**
  * @brief add a DNS query result to the history for this resolver.
  */
void OpenNICResolverPoolItem::addToHistory(OpenNICDnsQuery* query)
{
	mHistory.prepend(query);
	pruneHistory();
}

void OpenNICResolverPoolItem::starting(OpenNICDnsQuery* query)
{
}

void OpenNICResolverPoolItem::finished(OpenNICDnsQuery* query)
{
	//fprintf(stderr,"finished\n");
	addToHistory(query);
}

void OpenNICResolverPoolItem::expired(OpenNICDnsQuery* query)
{
}

/**
  * @brief run the resolver test
  */
void OpenNICResolverPoolItem::test()
{
	//fprintf(stderr,"test\n");
	new OpenNICDnsQuery(this,hostAddress(),OpenNICSystem::randomDomain()); /* launch a new query */
	resetQueryTimer();

}

/**
  * @brief reset the query timer
  */
void OpenNICResolverPoolItem::resetQueryTimer()
{
	if (mQueryIntervalTimer >= 0 )
	{
		killTimer(mQueryIntervalTimer);
		mQueryIntervalTimer=-1;
	}
	mQueryIntervalTimer = startTimer(OpenNICSystem::random(RANDOM_INTERVAL_MIN,RANDOM_INTERVAL_MAX));
}

/**
  * @brief get here once in a while to run a test
  */
void OpenNICResolverPoolItem::timerEvent(QTimerEvent *e)
{
	if ( e->timerId() == mQueryIntervalTimer )
	{
		test();
	}
}




