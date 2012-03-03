/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicdnsquery.h"

#define inherited QObject

OpenNICDnsQuery::OpenNICDnsQuery(QObject *parent)
: inherited(parent)
, mError(DNS_OK)
, mQueryType(DNS_A_RECORD)
, mTid(0)
{
}

OpenNICDnsQuery::OpenNICDnsQuery(const OpenNICDnsQuery& other)
: inherited(NULL)
{
	copy(other);
}

OpenNICDnsQuery::~OpenNICDnsQuery()
{
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
	mError			= other.mError;
	mQueryType		= other.mQueryType;
	mTid			= other.mTid;
	mStartTime		= other.mStartTime;
	mEndTime		= other.mEndTime;
	mExpireTime		= other.mExpireTime;
	mName			= other.mName;
	mAddr			= other.mAddr;
	mMxName			= other.mMxName;
	return *this;
}

/**
  * @brief calculate the latency
  */
quint64 OpenNICDnsQuery::latency()
{
	return mStartTime.msecsTo(mEndTime);
}

/**
  * @brief set the end time and emit the finished signal
  */
void OpenNICDnsQuery::setEndTime(QDateTime endTime)
{
	mEndTime = endTime;
	emit finished(this);
}






