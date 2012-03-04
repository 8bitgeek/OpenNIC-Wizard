/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicdnsquerylistener.h"
#include <QMultiHash>

#define inherited QObject

OpenNICDnsQueryListener::OpenNICDnsQueryListener(QObject *parent)
: inherited(parent)
, mMaxHistoryDepth(MAX_HISTORY_DEPTH)
{
}

OpenNICDnsQueryListener::OpenNICDnsQueryListener(const OpenNICDnsQueryListener& other)
: inherited()
, mMaxHistoryDepth(MAX_HISTORY_DEPTH)
{
	copy(other);
}

OpenNICDnsQueryListener::~OpenNICDnsQueryListener()
{
	for(int n=0; n < mHistory.count(); n++)
	{
		delete mHistory.takeAt(n);
	}
	mHistory.clear();
}

OpenNICDnsQueryListener& OpenNICDnsQueryListener::copy(const OpenNICDnsQueryListener& other)
{
	if ( &other != this )
	{
		for(int n=0; n < other.mHistory.count(); n++)
		{
			OpenNICDnsQuery* query = new OpenNICDnsQuery(this,*(other.mHistory[n]));
			mHistory.append(query);
		}
		mMaxHistoryDepth	= other.mMaxHistoryDepth;
	}
	return *this;
}


/**
  * @brief set the maximum history depth.
  */
void OpenNICDnsQueryListener::setMaxHistoryDepth(int maxHistoryDepth)
{
	if (maxHistoryDepth>=1)
	{
		mMaxHistoryDepth = maxHistoryDepth;
	}
	pruneHistory();
}

/**
  * @brief prune history record back to the limit.
  */
void OpenNICDnsQueryListener::pruneHistory()
{
	/* prune... */
	while(mHistory.count()>0 && mHistory.count()>maxHistoryDepth())
	{
		delete mHistory.takeFirst();
	}
}

/**
  * @brief add a DNS query result to the history for this resolver.
  */
void OpenNICDnsQueryListener::addToHistory(OpenNICDnsQuery* query)
{
	mHistory.prepend(query);
	pruneHistory();
}

/**
  * @brief reset all internals to default state.
  */
void OpenNICDnsQueryListener::clear()
{
	mHistory.clear();
	mMaxHistoryDepth=MAX_HISTORY_DEPTH;
}
