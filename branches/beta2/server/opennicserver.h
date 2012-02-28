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

#define	VERSION_STRING				"0.2.1"

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
        QMutex&                 processMutex()  {return mProcessMutex;}
        bool					process(QTcpSocket* client);

	signals:
		void					quit();

	protected:
		QMap<QString,QVariant>	mapServerStatus();
		void					mapClientRequest(QMap<QString,QVariant>& map);
		int						initializeServer();
		int						initializeResolvers();
		int						updateDNS(int resolver_count);
		virtual void			timerEvent(QTimerEvent* e);

	protected slots:
		void					newConnection();
		void					readSettings();
		void					writeSettings();

	private:
		bool					mEnabled;					/** service status */
		int						mStartTimer;
		int						mRefreshTimer;
		int						mFastTimer;
		/** TCP service */
		QTcpServer				mServer;					/** the localhost TCP server */
        QMutex                  mProcessMutex;              /** protects the data */
        QList<OpenNICSession*>  mSessions;                  /** active sessions */
		/** settings **/
		int						mTcpListenPort;				/** the TCP listen port */
		QString					mLogFile;					/** the log file */
		QStringList				mResolverCache;				/** most recently selected resolver cache */
		int						mResolverCacheSize;			/** the number of resolvers to keep in the cache (and apply to the O/S) */
		int						mResolverRefreshRate;		/** the resolver refresh rate (apply cache to O/S) */
		OpenNICResolverPool		mResolverPool;				/** the active resolver pool */
};

#endif // OPENNICSERVER_H
