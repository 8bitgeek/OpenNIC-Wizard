/*
 *   This file is a part of OpenNIC Wizard
 *   Copyright (C) 2012-2015  Mike Sharkey <mike@8bitgeek.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "opennicdnsquerylistener.h"
#include "opennicserver.h"
#include <QMultiHash>

#define inherited QObject

OpenNICDnsQueryListener::OpenNICDnsQueryListener(QObject *parent)
: inherited(parent)
, mMaxQueryDepth(MAX_QUERY_DEPTH)
{
}

OpenNICDnsQueryListener::OpenNICDnsQueryListener(const OpenNICDnsQueryListener& other)
: inherited()
, mMaxQueryDepth(MAX_QUERY_DEPTH)
{
	copy(other);
}

OpenNICDnsQueryListener::~OpenNICDnsQueryListener()
{
	for(int n=0; n < mQueries.count(); n++)
	{
		delete mQueries.takeAt(n);
	}
	mQueries.clear();
}

OpenNICDnsQueryListener& OpenNICDnsQueryListener::copy(const OpenNICDnsQueryListener& other)
{
	if ( &other != this )
	{
		for(int n=0; n < other.mQueries.count(); n++)
		{
			OpenNICDnsQuery* otherQuery = other.mQueries.at(n);
			OpenNICDnsQuery* query = new OpenNICDnsQuery(this,*otherQuery);
			mQueries.append(query);
		}
		mMaxQueryDepth = other.mMaxQueryDepth;
	}	return *this;
}

/**
  * @brief retrieve the status
  */
OpenNICDnsQueryListener::Status OpenNICDnsQueryListener::status()
{
	if (mQueries.count())
	{
		int failures=0;
		int successes=0;
		for(int n=0; n < (mQueries.count() <= 3 ? mQueries.count(): 3); n++)
		{
			OpenNICDnsQuery* query = mQueries[n];
			switch(query->error())
			{
			case OpenNICDnsQuery::DNS_OK:
				++successes;
				break;
			case OpenNICDnsQuery::DNS_DOES_NOT_EXIST:
				++failures;
				break;
			case OpenNICDnsQuery::DNS_TIMEOUT:
				++failures;
				break;
			default:
			case OpenNICDnsQuery::DNS_ERROR:
				++failures;
				break;
			}
		}
		if (failures==0 && successes > 0)
		{
			return Green;
		}
		if (successes-failures >= 1)
		{
			return Yellow;
		}
	}
	return Red;
}


/**
  * @brief set the maximum history depth.
  */
void OpenNICDnsQueryListener::setMaxQueryDepth(int maxHistoryDepth)
{
	if (maxHistoryDepth>=1)
	{
		mMaxQueryDepth = maxHistoryDepth;
	}
	pruneQueries();
}

/**
  * @brief prune history record back to the limit.
  */
void OpenNICDnsQueryListener::pruneQueries()
{
	/* prune... */
	while(mQueries.count()>0 && mQueries.count()>maxHistoryDepth())
	{
		delete mQueries.takeLast();
	}
}

/**
  * @brief add a DNS query result to the history for this resolver.
  */
void OpenNICDnsQueryListener::addToQueries(OpenNICDnsQuery* query)
{
	mQueries.prepend(query);
	pruneQueries();
}

/**
  * @brief reset all internals to default state.
  */
void OpenNICDnsQueryListener::clear()
{
	mQueries.clear();
	mMaxQueryDepth=MAX_QUERY_DEPTH;
}

void OpenNICDnsQueryListener::starting(OpenNICDnsQuery* query)
{
	//OpenNICServer::log("starting "+query->resolver().toString()+" : "+query->name().toString());
}

void OpenNICDnsQueryListener::finished(OpenNICDnsQuery* query)
{
	//OpenNICServer::log("finished "+query->resolver().toString()+" : "+query->name().toString());
}

void OpenNICDnsQueryListener::expired(OpenNICDnsQuery* query)
{
	//OpenNICServer::log("expired "+query->resolver().toString()+" : "+query->name().toString());
	query->setError(OpenNICDnsQuery::DNS_TIMEOUT);
}

