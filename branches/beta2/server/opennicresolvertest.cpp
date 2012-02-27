/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicresolvertest.h"

#define inherited OpenNICDnsClient

OpenNICResolverTest::OpenNICResolverTest(QObject *parent)
: inherited(parent)
{
	setInterval(10);
}

OpenNICResolverTest::~OpenNICResolverTest()
{
	killTimer(mSecondTimer);
	for(int n=0; n < mQueries.count(); n++)
	{
		delete mQueries.at(n);
	}
	mQueries.clear();
}

/**
  * @brief Set interval
  * @param sedconds or stop time if <= 0
  */
void OpenNICResolverTest::setInterval(int seconds)
{
	if ( mSecondTimer >= 0 )
	{
		killTimer(mSecondTimer);
		mSecondTimer=(-1);
	}
	if ( seconds > 0 )
	{
		mSecondTimer = startTimer(seconds*1000);
	}
}

/**
  * @brief Dispose the query.
  */
void OpenNICResolverTest::dispose(query* q)
{
	int n = mQueries.indexOf(q);
	if ( n >= 0 )
	{
		delete mQueries.takeAt(0);
	}
}

/**
  * @param Append the query
  */
void OpenNICResolverTest::append(query* q)
{
	mQueries.append(q);
}

/**
  * @brief Find a query based on a context from a reply.
  */
OpenNICResolverTest::query* OpenNICResolverTest::find(void* context)
{
	int n = mQueries.indexOf((query*)context);
	if ( n >= 0 )
	{
		return mQueries.at(n);
	}
	return NULL;
}

/**
  * @brief Purge old queries
  */
void OpenNICResolverTest::purge()
{
	QDateTime now = QDateTime::currentDateTime();
	for(int n=0; n < mQueries.count(); n++ )
	{
		query* q = mQueries.at(n);
		if ( q->start.secsTo(now) > PURGE_TIMEOUT )
		{
			delete mQueries.takeAt(n--);
		}
	}
}

/**
  * @brief Test a resolver.
  */
void OpenNICResolverTest::resolve(QHostAddress addr,QString name,quint16 port)
{
	query* q = new query;
	q->addr			= addr;
	q->name			= name;
	q->start		= QDateTime::currentDateTime();
	q->latency		= 0;
	q->error		= 0;
	append(q);
	lookup(addr,name,OpenNICDnsClient::DNS_A_RECORD,q,port);
}

/**
  * get here on reply data or timeotu
  */
void OpenNICResolverTest::reply(dns_cb_data& rdata)
{
	query* q = find(rdata.context);
	if ( q != NULL )
	{
		q->latency	= q->start.msecsTo(QDateTime::currentDateTime());
		q->addr		= rdata.addr;
		q->error	= rdata.error;
		q->mxName	= rdata.mxName;
	}
}

void OpenNICResolverTest::timerEvent(QTimerEvent *e)
{
	inherited::timerEvent(e);
	if ( mSecondTimer == e->timerId() )
	{
		purge();
	}
}

