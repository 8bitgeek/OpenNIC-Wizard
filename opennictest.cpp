/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennictest.h"

#define inherited QObject

OpenNICTest::OpenNICTest(QObject *parent)
: inherited(parent)
{
	mDns = new OpenNICDns(this);
	QObject::connect(mDns,SIGNAL(reply(dns_cb_data&)),this,SLOT(reply(OpenNICDns::dns_cb_data&)));
	mSecondTimer = startTimer(1000*10);
}

OpenNICTest::~OpenNICTest()
{
	delete mDns;
	mDns = NULL;
	killTimer(mSecondTimer);
	for(int n=0; n < mQueries.count(); n++)
	{
		delete mQueries.at(n);
	}
	mQueries.clear();
}

/**
  * @brief Dispose the query.
  */
void OpenNICTest::dispose(query* q)
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
void OpenNICTest::append(query* q)
{
	mQueries.append(q);
}

/**
  * @brief Find a query based on a context from a reply.
  */
OpenNICTest::query* OpenNICTest::find(void* context)
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
void OpenNICTest::purge()
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
void OpenNICTest::resolve(QHostAddress addr,QString name,quint16 port)
{
	query* q = new query;
	q->addr			= addr;
	q->name			= name;
	q->start		= QDateTime::currentDateTime();
	q->latency		= 0;
	q->error		= 0;
	append(q);
	dns()->lookup(addr,name,OpenNICDns::DNS_A_RECORD,q,port);
}

void OpenNICTest::reply(OpenNICDns::dns_cb_data &rdata)
{
	query* q = find(rdata.context);
	if ( q != NULL )
	{
		q->latency	= q->start.msecsTo(QDateTime::currentDateTime());
		q->addr		= rdata.addr;
		q->error	= rdata.error;
		q->mxName	= rdata.mxName;
		emit queryResult(q);
	}
}

void OpenNICTest::timerEvent(QTimerEvent *e)
{
	inherited::timerEvent(e);
	if ( mSecondTimer == e->timerId() )
	{
		purge();
	}
}

