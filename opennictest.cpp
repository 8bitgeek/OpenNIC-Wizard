/*
 * Copyright (c) 2012 Mike Sharkey <mike@pikeaero.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennictest.h"

#define inherited QThread

OpenNICTest::OpenNICTest(OpenNICDns *parent)
: inherited(parent)
{
	QObject::connect(dns(),SIGNAL(reply(dns_cb_data&)),this,SLOT(reply(OpenNICDns::dns_cb_data&)));
	mSecondTimer = startTimer(1000*10);
}

OpenNICTest::~OpenNICTest()
{
}

void OpenNICTest::run()
{
	exec();
}

/**
  * @brief Dispose the query.
  */
void OpenNICTest::dispose(query* q)
{
	mQueriesMutex.lock();
	int n = mQueries.indexOf(q);
	if ( n >= 0 )
	{
		delete mQueries.takeAt(0);
	}
	mQueriesMutex.unlock();
}

/**
  * @param Append the query
  */
void OpenNICTest::append(query* q)
{
	mQueriesMutex.lock();
	mQueries.append(q);
	mQueriesMutex.unlock();
}

/**
  * @brief Find a query based on a context from a reply.
  */
OpenNICTest::query* OpenNICTest::find(void* context)
{
	mQueriesMutex.lock();
	int n = mQueries.indexOf((query*)context);
	if ( n >= 0 )
	{
		mQueriesMutex.unlock();
		return mQueries.at(n);
	}
	mQueriesMutex.unlock();
	return NULL;
}

/**
  * @brief Purge old queries
  */
void OpenNICTest::purge()
{
	QDateTime now = QDateTime::currentDateTime();
	mQueriesMutex.lock();
	for(int n=0; n < mQueries.count(); n++ )
	{
		query* q = mQueries.at(n);
		if ( q->start.secsTo(now) > PURGE_TIMEOUT )
		{
			delete mQueries.takeAt(n--);
		}
	}
	mQueriesMutex.unlock();
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

