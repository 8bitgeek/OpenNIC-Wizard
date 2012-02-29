/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICSERVER_H
#define OPENNICSERVER_H

#include <QString>
#include <QStringList>
#include <QTimerEvent>
#include <QAction>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QVariant>
#include <QMutex>
#include <QList>
#include "opennicresolverpool.h"

#define	VERSION_STRING				"0.2.4"

class OpenNICSession;
class OpenNICServer : public QObject
{
	Q_OBJECT

	public:
		explicit OpenNICServer(QObject *parent = 0);
		virtual ~OpenNICServer();

		QString					copyright();
		QString					license();

		/** service layer interface */
		void					pause()			{mEnabled = false;}
		void					resume()		{mEnabled = true;}
		bool					isListening()	{return mServer.isListening();}
		quint16					serverPort()	{return mServer.serverPort();}

	public slots:
		void					log(QString msg);
		void					logPurge();
		void					runOnce();

	signals:
		void					packet(QMap<QString,QVariant> packet);
		void					quit();

	protected:
		void					pruneLog();
		void					setRefreshTimerPeriod(int refreshTimerPeriod);
		int						refreshTimerPeriod() {return mRefreshTimerPeriod;}
		bool					testResolverCache();
		void					coldBoot();
		int						bootstrapResolvers();
		void					refreshResolvers(bool force=false);
		void					announcePackets();
		void					purgeDeadSesssions();
		QMap<QString,QVariant>	makeServerPacket();
		int						initializeServer();
		int						updateDNS(int resolver_count);
		virtual void			timerEvent(QTimerEvent* e);

	protected slots:
		void					sessionPacket(OpenNICSession* session, QMap<QString,QVariant> packet);
		void					newConnection();
		void					readSettings();
		void					writeSettings();

	private:
		QStringList				mLog;						/** log history */
		bool					mEnabled;					/** service status */
		int						mRefreshTimer;
		int						mFastTimer;
		bool					mResolversInitialized;      /** resolvers have been initialized */
		/** TCP service */
		QTcpServer				mServer;					/** the localhost TCP server */
        QList<OpenNICSession*>  mSessions;                  /** active sessions */
		/** settings **/
		int						mRefreshTimerPeriod;		/** the refresh timer period in minutes */
		int						mTcpListenPort;				/** the TCP listen port */
		int						mResolverCacheSize;			/** the number of resolvers to keep in the cache (and apply to the O/S) */
		OpenNICResolverPool		mResolverPool;				/** the comlpete resolver pool */
		OpenNICResolverPool		mResolverCache;				/** the active resolver pool */
};

#endif // OPENNICSERVER_H
