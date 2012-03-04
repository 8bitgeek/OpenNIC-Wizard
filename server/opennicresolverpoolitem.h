/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICRESOLVERPOOLITEM_H
#define OPENNICRESOLVERPOOLITEM_H

#include <QObject>
#include <QHostAddress>
#include <QDateTime>
#include <QString>
#include <QTimerEvent>

#include "opennicdnsquery.h"
#include "opennicdnsquerylistener.h"

#define RESOLVER_SAMPLE_STORAGE_LIMIT		10			/* number of samples to remember for averaging */

class OpenNICResolverPoolItem : public OpenNICDnsQueryListener
{
	Q_OBJECT
	public:
		OpenNICResolverPoolItem(QObject *parent = 0);
		OpenNICResolverPoolItem(bool active, QObject *parent = 0);
		OpenNICResolverPoolItem(QHostAddress hostAddress, QString kind="", QObject* parent=NULL);
		OpenNICResolverPoolItem(const OpenNICResolverPoolItem& other);
		virtual ~OpenNICResolverPoolItem();
		OpenNICResolverPoolItem&	copy(const OpenNICResolverPoolItem& other);
		OpenNICResolverPoolItem&	operator=(const OpenNICResolverPoolItem& other);
		bool						operator==(OpenNICResolverPoolItem &other);
		bool						operator!=(OpenNICResolverPoolItem &other);
		bool						operator>(OpenNICResolverPoolItem &other);
		bool						operator<(OpenNICResolverPoolItem &other);
		bool						operator>=(OpenNICResolverPoolItem &other);
		bool						operator<=(OpenNICResolverPoolItem &other);
		QHostAddress&				hostAddress()		{return mHostAddress;}
		int							testCount();
		int							replyCount();
		int							timeoutCount();
		int							lastLatency();
		double						averageLatency();
		QDateTime					lastReply();
		QDateTime					lastTimeout();
		QString						lastFault();
		bool						alive();
		double						score();
		void						setScore(double score);
		QString&					kind()				{return mKind;}
		void						setKind(QString kind) {mKind=kind;}
		OpenNICDnsQuery*			mostRecentQuery();
		QString&					toString();
	protected slots:
		virtual void				starting(OpenNICDnsQuery* query);
		virtual void				finished(OpenNICDnsQuery* query);
		virtual void				expired(OpenNICDnsQuery* query);
	protected:
		virtual	void				test();
		virtual void				timerEvent(QTimerEvent *);
	public slots:
		virtual void				clear();
	private slots:
		void						resetQueryTimer();
	private:
		double						mScore;				/* the score realtive to other resolvers in the pool */
		QHostAddress				mHostAddress;		/* host address wrapper */
		QString						mKind;				/* the kind of resolver */
		int							mQueryIntervalTimer;/* interval timer */
		int							mBootstrapTicks;	/* higher frequency intervals during the bootstrap ticks */
		QString						mString;			/* for returning a string reference toString() */
};


#endif // OPENNICRESOLVERPOOLITEM_H
