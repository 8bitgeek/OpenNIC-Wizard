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
#include "opennicresolvertest.h"
#include "opennicsystem.h"

#define		RANDOM_INTERVAL_MIN			10		/* seconds */
#define		RANDOM_INTERVAL_MAX			(10*60)	/* seconds */

#define inherited OpenNICDnsClient

#define TIMER_RESOLUTION	5	/* seconds */

OpenNICResolverTest::OpenNICResolverTest(bool active, QObject *parent)
: inherited(active, parent)
, mTimerInterval(0)
, mTimerCount(0)
{
	if ( isActive() )
	{
		mSecondTimer = startTimer(1000*TIMER_RESOLUTION);
	}
	setInterval(10); /* a small delay for bootstrap */
}

OpenNICResolverTest::~OpenNICResolverTest()
{
	killTimer(mSecondTimer);
}

/**
  * @brief Set interval
  * @param sedconds or stop time if <= 0
  */
void OpenNICResolverTest::setInterval(int seconds)
{
	mTimerInterval = seconds;
}

/**
  * get here on reply data or timeout
  */
void OpenNICResolverTest::reply(OpenNICDnsQuery& query)
{
	/* time stamp the query reply */
	QDateTime endTime = QDateTime::currentDateTime();
	query.setEndTime(endTime);
}

/**
  * @brief Test a resolver.
  */
void OpenNICResolverTest::resolve(QHostAddress addr,OpenNICDomainName name,quint16 port)
{
	QDateTime startTime = QDateTime::currentDateTime();
	OpenNICDnsQuery* q = new OpenNICDnsQuery;
	q->setAddr(addr);
	q->setName(name);
	q->setStartTime(startTime);
	lookup(addr,name,OpenNICDnsQuery::DNS_A_RECORD,port);
}

void OpenNICResolverTest::timerEvent(QTimerEvent *e)
{
	inherited::timerEvent(e);
	if ( mSecondTimer == e->timerId() )
	{
		purge();
		mTimerCount += TIMER_RESOLUTION;
		if (mTimerCount >= mTimerInterval)
		{
			if ( isActive() )
			{
				test();
			}
			setInterval(OpenNICSystem::random(RANDOM_INTERVAL_MIN,RANDOM_INTERVAL_MAX)); /* set next random interval */
			mTimerCount=0;
		}
	}
}

