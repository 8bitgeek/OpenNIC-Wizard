/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICDNSQUERYLISTENER_H
#define OPENNICDNSQUERYLISTENER_H

#include <QObject>

#include "opennicdnsquery.h"

#define		MAX_QUERY_DEPTH			30				/* the default maximum history depth */

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
	public slots:
		virtual void					clear();
		void							setMaxQueryDepth(int maxQueryDepth);
	protected slots:
		virtual void					starting(OpenNICDnsQuery* query) = 0;
		virtual void					finished(OpenNICDnsQuery* query) = 0;
		virtual void					expired(OpenNICDnsQuery* query) = 0;
	protected:
		virtual	void					pruneQueries();
		virtual	void					addToQueries(OpenNICDnsQuery* query);
		QList<OpenNICDnsQuery*>&		queries()			{return mQueries;}
	private:
		QList<OpenNICDnsQuery*>			mQueries;			/* maintain a recent history */
		int								mMaxQueryDepth;		/* the maximum depth of history */
};

#endif // OPENNICDNSQUERYLISTENER_H
