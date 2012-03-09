/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicresolver.h"
#include "opennicsystem.h"
#include "opennicserver.h"

#define		RANDOM_INTERVAL_MIN			(10*1000)		/* milliseconds */
#define		RANDOM_INTERVAL_MAX			((10*60)*1000)	/* milliseconds */
#define		MAX_TIMEOUT					(10*1000)		/* milliseconds */
#define		BOOTSTRAP_TIMER_TICKS		3

#define inherited OpenNICDnsQueryListener

OpenNICResolver::OpenNICResolver(QObject* parent)
: inherited(parent)
, mRefCount(0)
, mScore(0.0)
, mQueryIntervalTimer(-1)
, mBootstrapTicks(0)
{
	clear();
}

OpenNICResolver::OpenNICResolver(QHostAddress hostAddress, QString kind, QObject* parent)
: inherited(parent)
, mRefCount(0)
, mScore(0.0)
, mQueryIntervalTimer(-1)
, mBootstrapTicks(0)
{
	clear();
	mHostAddress = hostAddress;
	mKind = kind;
	test();
}

OpenNICResolver::OpenNICResolver(const OpenNICResolver& other)
: inherited(NULL)
, mRefCount(0)
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
OpenNICResolver::~OpenNICResolver()
{
	clear();
}

/**
  * @brief copy from another.
  * @return a self reference
  */
OpenNICResolver& OpenNICResolver::copy(const OpenNICResolver& other)
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
  * @brief increment reference count
  */
int OpenNICResolver::incRef()
{
	return ++mRefCount;
}

/**
  * @brief decrement reference count
  */
int OpenNICResolver::decRef()
{
	if ( --mRefCount <= 0 )
	{
		this->deleteLater();
	}
	return mRefCount;
}

/**
  * @return the ref count
  */
int OpenNICResolver::refCount()
{
	return mRefCount;
}

/**
  * @return the host name
  */
QHostAddress& OpenNICResolver::hostAddress()
{
	return mHostAddress;
}

/**
  * @return the kind (class) of resolver
  */
QString& OpenNICResolver::kind()
{
	return mKind;
}

/**
  * @brief set the kind (class) of resolver
  */
void OpenNICResolver::setKind(QString kind)
{
	mKind=kind;
}

/**
  * @brief equality operator
  */
bool OpenNICResolver::operator==(OpenNICResolver &other)
{
	return score() == other.score();
}

/**
  * @brief unequality operator
  */
bool OpenNICResolver::operator!=(OpenNICResolver &other)
{
	return score() != other.score();
}

/**
  * @brief greater-than operator (greater means less latency)
  */
bool OpenNICResolver::operator>(OpenNICResolver &other)
{
	return score() > other.score();
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolver::operator<(OpenNICResolver &other)
{
	return score() < other.score();
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolver::operator>=(OpenNICResolver &other)
{
	return score() >= other.score();
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolver::operator<=(OpenNICResolver &other)
{
	return score() <= other.score();
}


/**
  * @brief assignment operator.
  */
OpenNICResolver& OpenNICResolver::operator=(const OpenNICResolver &other)
{
	return copy(other);
}

/**
  * @brief get the date/time of the last reply in the queue
  */
QDateTime OpenNICResolver::lastReply()
{
	QDateTime rc;
	for(int n=0; n < queries().count(); n++)
	{
		OpenNICDnsQuery* query = queries().at(n);
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
QDateTime OpenNICResolver::lastTimeout()
{
	QDateTime rc;
	for(int n=0; n < queries().count(); n++)
	{
		OpenNICDnsQuery* query = queries().at(n);
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
QString OpenNICResolver::lastFault()
{
	QString rc;
	if (queries().count())
	{
		OpenNICDnsQuery* query = queries().at(0);
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
bool OpenNICResolver::alive()
{
	int deadCount=0;
	for(int n=0; n < queries().count(); n++)
	{
		OpenNICDnsQuery* query = queries().at(n);
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
  * @brief <status>;<score>;<kind>;<hostAddress>;
  */
QString& OpenNICResolver::toString()
{
	mString.clear();
	switch(status())
	{
	default:
	case Red:		mString += "R";		break;
	case Yellow:	mString += "Y";		break;
	case Green:		mString += "G";		break;
	}
	mString += ";";
	mString += QString("%1").arg(score(),7,'f',3,'0') + ";";
	mString += kind() + ";";
	mString += hostAddress().toString() + ";";
	return mString;
}

/**
  * @brief reset all internals to default state.
  */
void OpenNICResolver::clear()
{
	inherited::clear();
	mHostAddress.clear();
	mKind.clear();
}

/**
  * @brief Return the number of tests which have been performed and are still on record in the history
  */
int OpenNICResolver::testCount()
{
	return queries().count();
}

/**
  * @brief replies are those dns queries that came back as either DNS_OK or DNS_DOES_NOT_EXIST meaning communication with the resolver was established.
  * @return the number of replies in the history for this resolver.
  */
int OpenNICResolver::replyCount()
{
	int count=0;
	int nQueries = queries().count();
	for(int n=0; n < nQueries; n++)
	{
		OpenNICDnsQuery* query = queries()[n];
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
int OpenNICResolver::timeoutCount()
{
	int count=0;
	int nQueries = queries().count();
	for(int n=0; n < nQueries; n++)
	{
		OpenNICDnsQuery* query = queries()[n];
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
int OpenNICResolver::lastLatency()
{
	int latency=-1.0; /* something very big if there are no samples */
	if ( queries().count() > 0 )
	{
		latency = queries()[0]->latency();
	}
	return latency;
}

/**
  * @return the average latency from the history buffer.
  */
double OpenNICResolver::averageLatency()
{
	double total=0.0;
	int nQueries = queries().count();
	if ( nQueries > 0 )
	{
		for(int n=0; n < nQueries; n++)
		{
			total += queries()[n]->latency();
		}
		return total/nQueries;
	}
	OpenNICServer::log("No Query History "+hostAddress().toString());
	return -1.0;
}

/**
  * @brief return a pointer to the last query pushed into the history
  */
OpenNICDnsQuery* OpenNICResolver::mostRecentQuery()
{
	if ( queries().count() > 0 )
	{
		OpenNICDnsQuery* query = queries().at(0);
		return query;
	}
	return NULL;
}

/**
  * @return true if domains within this NIC have been resolved by this resolver
  */
bool OpenNICResolver::resolvesNIC(QString nic)
{
	int nQueries = queries().count();
	for(int n=0; n < nQueries; n++)
	{
		OpenNICDnsQuery* query = queries().at(n);
		if (query->name().dnsService() == nic && query->error() == OpenNICDnsQuery::DNS_OK)
		{
			return true;
		}
	}
	return false;
}

/**
  * @brief calculate the score
  */
double OpenNICResolver::score()
{
	return mScore;
}

/**
  * @brief return the score
  */
void OpenNICResolver::setScore(double score)
{
	mScore=score;
}

void OpenNICResolver::starting(OpenNICDnsQuery* query)
{
	inherited::starting(query);
}

void OpenNICResolver::finished(OpenNICDnsQuery* query)
{
	inherited::finished(query);
}

void OpenNICResolver::expired(OpenNICDnsQuery* query)
{
	inherited::expired(query);
}

/**
  * @brief run the resolver test
  */
void OpenNICResolver::test()
{
	addToQueries(new OpenNICDnsQuery(this,hostAddress(),OpenNICSystem::randomDomain(),QDateTime::currentDateTime().addMSecs(MAX_TIMEOUT))); /* launch a new query */
	resetQueryTimer();
}

/**
  * @brief reset the query timer
  */
void OpenNICResolver::resetQueryTimer()
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
void OpenNICResolver::timerEvent(QTimerEvent *e)
{
	if ( e->timerId() == mQueryIntervalTimer )
	{
		test();
	}
}




