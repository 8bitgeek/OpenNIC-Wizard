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
		QList<OpenNICDnsQuery*>&	history()			{return mHistory;}
		int							historyDepth()		{return mHistory.count();}
		int							maxHistoryDepth()	{return mMaxHistoryDepth;}
		QString&					toString();
	protected slots:
		virtual void				starting(OpenNICDnsQuery* query);
		virtual void				finished(OpenNICDnsQuery* query);
		virtual void				expired(OpenNICDnsQuery* query);
	protected:
		virtual	void				pruneHistory();
		virtual	void				addToHistory(OpenNICDnsQuery* query);
		virtual	void				test();
		virtual void				timerEvent(QTimerEvent *);
	public slots:
		void						setMaxHistoryDepth(int maxHistoryDepth);
		void						clear();
	private slots:
		void						resetQueryTimer();
	private:
		double						mScore;				/* the score realtive to other resolvers in the pool */
		int							mMaxHistoryDepth;	/* the maximum depth of history */
		QList<OpenNICDnsQuery*>		mHistory;			/* maintain a recent history */
		QHostAddress				mHostAddress;		/* host address wrapper */
		QString						mKind;				/* the kind of resolver */
		int							mQueryIntervalTimer;/* interval timer */
		QString						mString;			/* for returning a string reference toString() */
};


#endif // OPENNICRESOLVERPOOLITEM_H
