/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicresolvertest.h"
#include "opennicsystem.h"

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
  * @brief Test a resolver.
  */
void OpenNICResolverTest::resolve(QHostAddress addr,QString name,quint16 port)
{
	dns_query* q = new dns_query;
	q->addr			= addr;
	q->name			= name;
	q->start		= QDateTime::currentDateTime();
	lookup(addr,name,OpenNICDnsClient::DNS_A_RECORD,port);
}

/**
  * get here on reply data or timeout
  */
void OpenNICResolverTest::reply(dns_query& rdata)
{
	rdata.latency = rdata.start.msecsTo(QDateTime::currentDateTime());
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
			setInterval(OpenNICSystem::random(10,60*10)); /* beween 10 seconds to 10 minutes */
			//setInterval(OpenNICSystem::random(10,60*5)); /* beween 10 seconds to 15 minutes */
			mTimerCount=0;
		}
	}
}

