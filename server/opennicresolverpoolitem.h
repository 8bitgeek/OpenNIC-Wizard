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

#include "opennicresolvertest.h"

#define RESOLVER_SAMPLE_STORAGE_LIMIT		10			/* number of samples to remember for averaging */

class OpenNICResolverPoolItem : public OpenNICResolverTest
{
	Q_OBJECT
	public:

		OpenNICResolverPoolItem(QObject *parent = 0);
		OpenNICResolverPoolItem(bool active, QObject *parent = 0);
		OpenNICResolverPoolItem(QHostAddress hostAddress, QString kind="", QObject* parent=NULL);
		OpenNICResolverPoolItem(const OpenNICResolverPoolItem& other);
		virtual ~OpenNICResolverPoolItem();

		OpenNICResolverPoolItem&	copy(const OpenNICResolverPoolItem& other);
		QHostAddress&				hostAddress()		{return mHostAddress;}
		int							testCount();
		int							replyCount();
		int							timeoutCount();
		int							lastLatency();
		double						averageLatency();

		QDateTime					lastReply();
		QDateTime					lastTimeout();
		QString						lastFault();

		int							inFlightTests()		{return mTestsinFlight;}
		bool						alive();

		QString&					kind()				{return mKind;}
		void						setKind(QString kind) {mKind=kind;}

		OpenNICResolverPoolItem&	operator=(const OpenNICResolverPoolItem& other);
		bool						operator==(OpenNICResolverPoolItem &other);
		bool						operator!=(OpenNICResolverPoolItem &other);
		bool						operator>(OpenNICResolverPoolItem &other);
		bool						operator<(OpenNICResolverPoolItem &other);
		bool						operator>=(OpenNICResolverPoolItem &other);
		bool						operator<=(OpenNICResolverPoolItem &other);

		QList<dns_query>&			history()			{return mHistory;}
		int							historyDepth()		{return mHistory.count();}
		int							maxHistoryDepth()	{return mMaxHistoryDepth;}
		QString&					toString();

	protected:
		virtual	void				addToHistory(dns_query& query);
		virtual void				test();
		virtual void				reply(dns_query& query);

	public slots:
		void						setMaxHistoryDepth(int maxHistoryDepth);
		void						clear();

	private:
		QList<dns_query>			mHistory;			/* maintain a recent history */
		int							mMaxHistoryDepth;	/* the maximum depth of history */
		QHostAddress				mHostAddress;		/* host address wrapper */
		QString						mKind;				/* the kind of resolver */
		int							mTestsinFlight;		/* number of tests in flight */
		QString						mString;			/* for returning a string reference toString() */

};


#endif // OPENNICRESOLVERPOOLITEM_H
