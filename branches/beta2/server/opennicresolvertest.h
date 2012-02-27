/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICRESOLVERTEST_H
#define OPENNICRESOLVERTEST_H

#include <QThread>
#include <QList>
#include <QMutex>
#include <QDateTime>
#include <QHostAddress>
#include <QTimerEvent>

#include "opennicdns.h"

#define PURGE_TIMEOUT	30		/* purge timeout in seconds */

class OpenNICTest : public QObject
{
	Q_OBJECT
	public:
		typedef struct
		{
			QHostAddress	addr;		/* The host ip address */
			QString			name;		/* The host name to query */
			QString			mxName;		/* The mx host name to query */
			QDateTime		start;		/* The start of the query */
			qint64			latency;	/* Latency in mulliseconds */
			int				error;		/* Error Code */
		} query;
		explicit OpenNICTest(QObject *parent=0);
		virtual ~OpenNICTest();
	protected slots:
		virtual void		reply(dns_cb_data& rdata);
	protected:
		OpenNICDns*			dns() {return mDns;}
		virtual void		resolve(QHostAddress addr, QString name, quint16 port=DEFAULT_DNS_PORT);
		virtual void		timerEvent(QTimerEvent* e);
	private:
		void				dispose(query* q);
		void				append(query* q);
		query*				find(void* context);
		void				purge();
		int					mSecondTimer;
		QList<query*>		mQueries;
		OpenNICDns*			mDns;
};

#endif // OPENNICRESOLVERTEST_H
