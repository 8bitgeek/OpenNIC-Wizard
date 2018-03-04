/*
 * This file is a part of OpenNIC Wizard
 * Copywrong (c) 2012-2018 Mike Sharkey
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 1776):
 * <mike@8bitgeek.net> wrote this file.
 * As long as you retain this notice you can do whatever you want with this
 * stuff. If we meet some day, and you think this stuff is worth it,
 * you can buy me a beer in return. ~ Mike Sharkey
 * ----------------------------------------------------------------------------
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

#include "opennicversion.h"
#include "opennicresolverpool.h"
#include "opennicnet.h"

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

		int						refreshPeriod();
		int						resolverCacheSize();
		void					setRefreshPeriod(int period);
		void					setResolverCacheSize(int size);

		static void				log(QString msg);

		static QString&			scoreRules()	{return mScoreRules;}
		static bool				scoreInternal()	{return mScoreInternal;}

	signals:
		void					quit();

	public slots:
		void					runOnce();

	private slots:
		void					pollKeyValue(QString& key, QVariant& value, bool& valid);
		void					dataReady(OpenNICNet* net);
		void					newConnection();
		void					readSettings();
		void					writeSettings();

	private:
		static void				logResolverPool(QString msg,OpenNICResolverPool& pool);
		static void				logPurge();
		bool					shouldReplaceWithProposed(OpenNICResolverPool& proposed);
		bool					replaceActiveResolvers(OpenNICResolverPool& proposed);
		void					updateRefreshTimerPeriod();
		void					pruneLog();
		bool					testResolverCache();
		void					coldBoot();
		int						bootstrapResolvers();
		void					refreshResolvers(bool force=false);
		void					purgeDeadSesssions();
		int						initializeServer();
		int						updateDNS(int resolver_count);
		void					delay(int seconds);
		virtual void			timerEvent(QTimerEvent* e);

	private:
		static QStringList		mLog;						/** log history */
		static QString			mScoreRules;				/** the score rules javascript text */
		static bool				mScoreInternal;				/** use internal scoring rules? */
		QList<OpenNICNet*>		mSessions;					/** active sessions */
		OpenNICResolverPool		mResolverPool;				/** the comlpete resolver pool */
		OpenNICResolverPool		mResolverCache;				/** the active resolver pool */
		QTcpServer				mServer;					/** the localhost TCP server */
		int						mRefreshTimerPeriod;		/** the refresh timer period in minutes */
		int						mResolverCacheSize;			/** the number of resolvers to keep in the cache (and apply to the O/S) */
		bool					mEnabled;					/** service status */
		int						mRefreshTimer;
		int						mFastTimer;
		int						mSecondTimer;				/** counts seconds */
		quint64					mSeconds;
		bool					mResolversInitialized;      /** resolvers have been initialized */
		int						mTcpListenPort;				/** the TCP listen port */
		bool					mUpdatingDNS;				/** in the processof updating the DNS */
		QString					mAsyncMessage;
		int						mBootstrapTimer;			/** short running HF timer for bootstrap */
		int						mBootstrapTicks;			/** number of botstrap ticks */
		bool					mInColdBoot;				/** Still cold booting */
};

#endif // OPENNICSERVER_H
