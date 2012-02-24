/*
 * Copyright (c) 2012 Mike Sharkey <mike@pikeaero.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICRESOLVER_H
#define OPENNICRESOLVER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QMultiHash>
#include <QTimerEvent>

#include "opennictest.h"

#define DEFAULT_THREAD_COUNT	1

class OpenNICResolver : public QObject
{
	Q_OBJECT
	public:
		explicit OpenNICResolver(QObject *parent = 0, int threadCount = DEFAULT_THREAD_COUNT);
		virtual ~OpenNICResolver();

		QStringList					defaultT1List();
		QString						addResolver(QString dns,int index);
		QStringList					getResolverList();
		QString						getSettingsText();

		void						setThreads(int count);
		int							threads() {return mThreads.count;}

	protected slots:
		void						insertResult(OpenNICTest::query* result);

	protected:
		virtual void				timerEvent(QTimerEvent *e);

	private:
		void						closeThreads();
		void						evaluateResolvers();
		QStringList					getBootstrapResolverList();
		void						initializeResolvers();
		QMultiHash<quint64,QString>	mResolvers;					/* latency (msecs) mapped to ip address */
		int							mMinuteTimer;				/* one minute resolution */
		QList<OpenNICTest*>			mThreads;					/* list of tester threads */

};

#endif // OPENNICRESOLVER_H
