/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicresolverpoolitem.h"

#define inherited OpenNICResolverTest

OpenNICResolverPoolItem::OpenNICResolverPoolItem(QObject* parent)
: inherited(parent)
{
	clear();
}

OpenNICResolverPoolItem::OpenNICResolverPoolItem(QHostAddress hostAddress, QString kind, QObject* parent)
: inherited(parent)
{
	clear();
	mHostAddress = hostAddress;
	mKind = kind;
}

OpenNICResolverPoolItem::OpenNICResolverPoolItem(const OpenNICResolverPoolItem& other)
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
  * @brief <hostAddress>;<testCount>;<replyCount>;<lastReply>;<lastTimeout>;<lastFault>;<kind>;
  */
QString OpenNICResolverPoolItem::toString()
{
	QString rc;
	rc += hostAddress().toString() + ";";
	rc += QString::number(testCount()) + ";";
	rc += QString::number(replyCount()) + ";";
	rc += lastReply().toString() + ";";
	rc += lastTimeout().toString() + ";";
	rc += lastFault() + ";";
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
	mLatencySamples;
	mLastReply.fromTime_t(0);
	mLastTimeout.fromTime_t(0);
	mLastFault;
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
	return 0.0;
}

/**
  * @brief get here on dns callback data
  */
void OpenNICResolverPoolItem::reply(dns_cb_data& data)
{
	/* FIXME - do stuff */
}





