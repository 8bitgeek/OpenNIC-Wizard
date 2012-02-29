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

#define	VERSION_STRING				"0.2.5"

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

		static int				refreshPeriod();
		static int				resolverCacheSize();
		static void				setRefreshPeriod(int period);
		static void				setResolverCacheSize(int size);

		static void				getPacket(QMap<QString,QVariant>& packet);

public slots:
		void					log(QString msg);
		void					logPurge();
		void					runOnce();

	signals:
		void					quit();

	protected:
		void					updateRefreshTimerPeriod();
		void					pruneLog();
		bool					testResolverCache();
		void					coldBoot();
		int						bootstrapResolvers();
		void					refreshResolvers(bool force=false);
		void					announcePackets();
		void					purgeDeadSesssions();
		QMap<QString,QVariant>&	makeServerPacket(QMap<QString,QVariant>& packet);
		int						initializeServer();
		int						updateDNS(int resolver_count);
		virtual void			timerEvent(QTimerEvent* e);

	protected slots:
		void					newConnection();
		void					readSettings();
		void					writeSettings();

	private:
		static QMutex			mLocker;					/** server data protection */
		static QMutex			mPacketLocker;
		static QMap<QString,QVariant>	mServerPacket;				/** the next packet to go out to the gui */
		static int				mPreviousRefreshTimerPeriod;
		static int				mRefreshTimerPeriod;		/** the refresh timer period in minutes */
		static int				mResolverCacheSize;			/** the number of resolvers to keep in the cache (and apply to the O/S) */
		QStringList				mLog;						/** log history */
		bool					mEnabled;					/** service status */
		int						mRefreshTimer;
		int						mFastTimer;
		bool					mResolversInitialized;      /** resolvers have been initialized */
		QTcpServer				mServer;					/** the localhost TCP server */
		QList<OpenNICSession*>  mSessions;                  /** active sessions */
		int						mTcpListenPort;				/** the TCP listen port */
		OpenNICResolverPool		mResolverPool;				/** the comlpete resolver pool */
		OpenNICResolverPool		mResolverCache;				/** the active resolver pool */
};

#endif // OPENNICSERVER_H
