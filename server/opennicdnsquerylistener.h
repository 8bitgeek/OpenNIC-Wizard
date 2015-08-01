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
#ifndef OPENNICDNSQUERYLISTENER_H
#define OPENNICDNSQUERYLISTENER_H

#include <QObject>

#include "opennicdnsquery.h"

#define		MAX_QUERY_DEPTH			10				/* the default maximum history depth */

class OpenNICDnsQueryListener : public QObject
{
    Q_OBJECT
	public:
		typedef enum
		{
			Red=0,							/* service is down */
			Yellow,							/* service may be down */
			Green							/* service is up */
		} Status;
		OpenNICDnsQueryListener(QObject *parent = 0);
		OpenNICDnsQueryListener(const OpenNICDnsQueryListener& other);
		virtual ~OpenNICDnsQueryListener();
		OpenNICDnsQueryListener&		copy(const OpenNICDnsQueryListener& other);
		int								queryDepth()		{return mQueries.count();}
		int								maxHistoryDepth()	{return mMaxQueryDepth;}
		Status							status();
		QList<OpenNICDnsQuery*>&		queries()			{return mQueries;}
	public slots:
		virtual void					clear();
		void							setMaxQueryDepth(int maxQueryDepth);
	protected slots:
		virtual void					starting(OpenNICDnsQuery* query);
		virtual void					finished(OpenNICDnsQuery* query);
		virtual void					expired(OpenNICDnsQuery* query);
	protected:
		virtual	void					pruneQueries();
		virtual	void					addToQueries(OpenNICDnsQuery* query);
	private:
		QList<OpenNICDnsQuery*>			mQueries;			/* maintain a recent history */
		int								mMaxQueryDepth;		/* the maximum depth of history */
};

#endif // OPENNICDNSQUERYLISTENER_H
