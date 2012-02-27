/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICSYSTEM_H
#define OPENNICSYSTEM_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QMultiMap>
#include <QTimerEvent>

#define OPENNIC_T1_BOOTSTRAP		"bootstrap.t1"
#define	OPENNIC_DOMAINS_BOOTSTRAP	"bootstrap.domains"

class OpenNICSystem : public QObject
{
	Q_OBJECT
	public:
		explicit OpenNICSystem(QObject *parent=0);
		virtual ~OpenNICSystem();

		QStringList					defaultT1List();
		QString						addResolver(QString dns,int index);
		QStringList					getResolvers();
		QString						getSettingsText();
		QMultiMap<quint64,QString>&	getResolverPool() {return mResolvers;}
		QStringList					getResolverPoolStringList();

	signals:
		void						resolverResult(QString ip, quint64 latency, int error);

	protected slots:
		void						insertResult(OpenNICTest::query* result);

	protected:
		virtual void				timerEvent(QTimerEvent *e);

	private:
		QStringList					getDomains();
		int							randInt(int low, int high);
		void						evaluateResolver();
		QStringList					getBootstrapResolverList();
		void						initializeResolvers();
		QMultiMap<quint64,QString>	mResolvers;					/* latency (msecs) mapped to ip address */
		int							mTimer;						/*  resolution */
		QStringList					mDomains;					/* domains to test with */
};

#endif // OPENNICSYSTEM_H
