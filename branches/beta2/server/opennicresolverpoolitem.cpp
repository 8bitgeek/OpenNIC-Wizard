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

#define inherited OpenNICResolverTest

OpenNICResolverPoolItem::OpenNICResolverPoolItem(QObject* parent)
: inherited(parent)
, mTests(0)
{
	clear();
}

OpenNICResolverPoolItem::OpenNICResolverPoolItem(QHostAddress hostAddress, QString kind, QObject* parent)
: inherited(parent)
, mTests(0)
{
	clear();
	mHostAddress = hostAddress;
	mKind = kind;
}

OpenNICResolverPoolItem::OpenNICResolverPoolItem(const OpenNICResolverPoolItem& other)
: inherited(NULL)
, mTests(0)
{
	copy(other);
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
  * @brief greater-than operator (greater means less latency)
  */
bool OpenNICResolverPoolItem::operator>(OpenNICResolverPoolItem &other)
{
	/* if one is alive and the other is not, then they are un-equal */
	if ( alive() != other.alive() )
	{
		return alive();
	}
	return averageLatency() < other.averageLatency();  /* less is more */
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolverPoolItem::operator<(OpenNICResolverPoolItem &other)
{
	/* if one is alive and the other is not, then they are un-equal */
	if ( alive() != other.alive() )
	{
		return !alive();
	}
	return averageLatency() > other.averageLatency();  /* less is more */
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolverPoolItem::operator>=(OpenNICResolverPoolItem &other)
{
	/* if one is alive and the other is not, then they are un-equal */
	if ( alive() != other.alive() )
	{
		return alive();
	}
	return averageLatency() <= other.averageLatency();  /* less is more */
}

/**
  * @brief less-than operator (greater means less latency)
  */
bool OpenNICResolverPoolItem::operator<=(OpenNICResolverPoolItem &other)
{
	/* if one is alive and the other is not, then they are un-equal */
	if ( alive() != other.alive() )
	{
		return !alive();
	}
	return averageLatency() >= other.averageLatency();  /* less is more */
}


/**
  * @brief assignment operator.
  */
OpenNICResolverPoolItem& OpenNICResolverPoolItem::operator=(const OpenNICResolverPoolItem &other)
{
	return copy(other);
}

/**
  * @brief convert to a formatted string
  * @brief <hostAddress>;<avgLatency>;<testCount>;<replyCount>;<lastReply>;<lastTimeout>;<lastFault>;<kind>;
  */
QString OpenNICResolverPoolItem::toString()
{
	QString rc;
	rc += hostAddress().toString() + ";";
	rc += QString::number((int)averageLatency()) + ";";
	rc += QString::number(testCount()) + ";";
	rc += QString::number(replyCount()) + ";";
	rc += lastReply().toString() + ";";
	rc += lastTimeout().toString() + ";";
	rc += lastFault() + ";";
	rc += kind() + ";";
	return rc;
}

/**
  * @brief copy from another.
  * @return a self reference
  */
OpenNICResolverPoolItem& OpenNICResolverPoolItem::copy(const OpenNICResolverPoolItem& other)
{
	mHostAddress	= other.mHostAddress;
	mTestCount		= other.mTestCount;
	mReplyCount		= other.mReplyCount;
	mLatencySamples	= other.mLatencySamples;
	mLastReply		= other.mLastReply;
	mLastTimeout	= other.mLastTimeout;
	mLastFault		= other.mLastFault;
	mKind			= other.mKind;
	return *this;
}

/**
  * @brief reset all internals to default state.
  */
void OpenNICResolverPoolItem::clear()
{
	mHostAddress.clear();
	mTestCount=0;
	mReplyCount=0;
	mLatencySamples.clear();
	mLastReply.fromTime_t(0);
	mLastTimeout.fromTime_t(0);
	mLastFault.clear();
	mKind.clear();
}

/**
  * @brief calculate the average latency
  */
double OpenNICResolverPoolItem::averageLatency()
{
	double total=0.0;
	if ( mLatencySamples.count() )
	{
		for(int n=0; n < mLatencySamples.count(); n++)
		{
			total += mLatencySamples.at(n);
		}
		return total/mLatencySamples.count();
	}
	return 10000;
}

/**
  * @brief get here on dns callback data
  */
void OpenNICResolverPoolItem::reply(dns_cb_data& data)
{
	if (mTests > 0)
	{
		QDateTime now = QDateTime::currentDateTime();
		if (data.error == OpenNICDnsClient::DNS_OK)
		{
			++mReplyCount;
			mLastReply = now;
			mLatencySamples.append(mTestBegin.msecsTo(now));
			mLastFault="DNS_OK " + data.addr.toString() + " " + data.name;
		}
		else if (data.error == OpenNICDnsClient::DNS_DOES_NOT_EXIST)
		{
			++mReplyCount;
			mLastReply = now;
			mLatencySamples.append(mTestBegin.msecsTo(now));
			mLastFault="DNS_DOES_NOT_EXIST " + data.name;
		}
		else if (data.error == OpenNICDnsClient::DNS_TIMEOUT)
		{
			mLastTimeout = now;
			mLastFault="DNS_TIMEOUT " + data.name;
		}
		else if (data.error == OpenNICDnsClient::DNS_ERROR)
		{
			mLastFault="DNS_ERROR " + data.name;
		}
		--mTests;
	}
	while(mLatencySamples.count() > 10)
	{
		mLatencySamples.takeAt(0);
	}
}

/**
  * @brief get here once in a while to run a test
  */
void OpenNICResolverPoolItem::test()
{
	if (mTests==0)
	{
		++mTests;
		++mTestCount;
		mTestBegin = QDateTime::currentDateTime();
		resolve(hostAddress(), OpenNICSystem::randomDomain());
	}
}




