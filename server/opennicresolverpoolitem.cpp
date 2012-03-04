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

#define		BIG_LATENCY					100000			/* for uninitialized latencies so they come up at bottom fo a sort */
#define		RANDOM_INTERVAL_MIN			(10*1000)		/* milliseconds */
#define		RANDOM_INTERVAL_MAX			((10*60)*1000)	/* milliseconds */
#define		MAX_TIMEOUT					(10*1000)		/* milliseconds */
#define		BOOTSTRAP_TIMER_TICKS		3

#define inherited OpenNICDnsQueryListener

OpenNICResolverPoolItem::OpenNICResolverPoolItem(QObject* parent)
: inherited(parent)
, mScore(0.0)
, mQueryIntervalTimer(-1)
, mBootstrapTicks(0)
{
	clear();
}

OpenNICResolverPoolItem::OpenNICResolverPoolItem(QHostAddress hostAddress, QString kind, QObject* parent)
: inherited(parent)
, mScore(0.0)
, mQueryIntervalTimer(-1)
, mBootstrapTicks(0)
{
	clear();
	mHostAddress = hostAddress;
	mKind = kind;
	test();
}

OpenNICResolverPoolItem::OpenNICResolverPoolItem(const OpenNICResolverPoolItem& other)
: inherited(NULL)
, mScore(0.0)
, mQueryIntervalTimer(-1)
, mBootstrapTicks(0)
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
  * @brief copy from another.
  * @return a self reference
  */
OpenNICResolverPoolItem& OpenNICResolverPoolItem::copy(const OpenNICResolverPoolItem& other)
{
	if ( &other != this )
	{
		inherited::copy(other);
		mScore				= other.mScore;
		mHostAddress		= other.mHostAddress;
		mKind				= other.mKind;

	}
	return *this;
}

/**
  * @brief equality operator
  */
bool OpenNICResolverPoolItem::operator==(OpenNICResolverPoolItem &other)
{
	return score() == other.score();
}

/**
  * @brief unequality operator
  */
bool OpenNICResolverPoolItem::operator!=(OpenNICResolverPoolItem &other)
{
	return score() != other.score();
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
	for(int n=0; n < history().count(); n++)
	{
		OpenNICDnsQuery* query = history().at(n);
		if ( query->error() == OpenNICDnsQuery::DNS_DOES_NOT_EXIST || query->error() == OpenNICDnsQuery::DNS_OK )
		{
			rc = query->endTime();
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
	for(int n=0; n < history().count(); n++)
	{
		OpenNICDnsQuery* query = history().at(n);
		if ( query->error() == OpenNICDnsQuery::DNS_TIMEOUT )
		{
			rc = query->endTime();
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
	if (history().count())
	{
		OpenNICDnsQuery* query = history().at(0);
		if (query->error() == OpenNICDnsQuery::DNS_OK)
		{
			rc="DNS_OK " + query->addr().toString() + " " + query->name().domainName()+" ("+query->name().dnsService()+")";
		}
		else if (query->error() == OpenNICDnsQuery::DNS_DOES_NOT_EXIST)
		{
			rc="DNS_DOES_NOT_EXIST " + query->name().domainName()+" ("+query->name().dnsService()+")";
		}
		else if (query->error() == OpenNICDnsQuery::DNS_TIMEOUT)
		{
			rc="DNS_TIMEOUT " + query->name().domainName()+" ("+query->name().dnsService()+")";
		}
		else if (query->error() == OpenNICDnsQuery::DNS_ERROR)
		{
			rc="DNS_ERROR " + query->name().domainName()+" ("+query->name().dnsService()+")";
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
	for(int n=0; n < history().count(); n++)
	{
		OpenNICDnsQuery* query = history().at(n);
		if ( query->error() == OpenNICDnsQuery::DNS_TIMEOUT || query->error() == OpenNICDnsQuery::DNS_ERROR )
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
  * @brief reset all internals to default state.
  */
void OpenNICResolverPoolItem::clear()
{
	inherited::clear();
	mHostAddress.clear();
	mKind.clear();
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
	int nHistory = history().count();
	for(int n=0; n < nHistory; n++)
	{
		OpenNICDnsQuery* query = history()[n];
		if ( query->error() == OpenNICDnsQuery::DNS_OK || query->error() == OpenNICDnsQuery::DNS_DOES_NOT_EXIST )
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
	int nHistory = history().count();
	for(int n=0; n < nHistory; n++)
	{
		OpenNICDnsQuery* query = history()[n];
		if ( query->error() == OpenNICDnsQuery::DNS_TIMEOUT )
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
	if ( history().count() > 0 )
	{
		latency = history()[0]->latency();
	}
	return latency;
}

/**
  * @return the average latency from the history buffer.
  */
double OpenNICResolverPoolItem::averageLatency()
{
	double total=0.0;
	int nHistory = history().count();
	if ( nHistory > 0 )
	{
		for(int n=0; n < nHistory; n++)
		{
			total += history()[n]->latency();
		}
		return total/nHistory;
	}
	return BIG_LATENCY;
}

/**
  * @brief return a pointer to the last query pushed into the history
  */
OpenNICDnsQuery* OpenNICResolverPoolItem::mostRecentQuery()
{
	if ( history().count() > 0 )
	{
		OpenNICDnsQuery* query = history().at(0);
		return query;
	}
	return NULL;
}

/**
  * @brief calculate the score
  */
double OpenNICResolverPoolItem::score()
{
	return mScore;
}

/**
  * @brief return the score
  */
void OpenNICResolverPoolItem::setScore(double score)
{
	mScore=score;
}

void OpenNICResolverPoolItem::starting(OpenNICDnsQuery* query)
{
	//fprintf(stderr,"starting\n");
}

void OpenNICResolverPoolItem::finished(OpenNICDnsQuery* query)
{
	//fprintf(stderr,"finished\n");
	//addToHistory(query);
}

void OpenNICResolverPoolItem::expired(OpenNICDnsQuery* query)
{
	fprintf(stderr,"expired %s : %s\n",query->resolver().toString().toAscii().data(), query->name().toString().toAscii().data());
}

/**
  * @brief run the resolver test
  */
void OpenNICResolverPoolItem::test()
{
	//fprintf(stderr,"test\n");
	addToHistory(new OpenNICDnsQuery(this,hostAddress(),OpenNICSystem::randomDomain(),QDateTime::currentDateTime().addMSecs(MAX_TIMEOUT))); /* launch a new query */
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
	if (mBootstrapTicks++ < BOOTSTRAP_TIMER_TICKS)
	{
		mQueryIntervalTimer = startTimer(OpenNICSystem::random(RANDOM_INTERVAL_MIN/4,RANDOM_INTERVAL_MAX/8));
	}
	else
	{
		mQueryIntervalTimer = startTimer(OpenNICSystem::random(RANDOM_INTERVAL_MIN,RANDOM_INTERVAL_MAX));
	}
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




