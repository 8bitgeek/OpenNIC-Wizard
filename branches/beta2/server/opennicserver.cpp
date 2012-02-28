/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicserver.h"
#include "openniclog.h"

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QEventLoop>
#include <QByteArray>
#include <QSettings>
#include <QMultiMap>
#include <QMap>
#include <QHostAddress>
#include <QTcpSocket>

#include "opennicsystem.h"
#include "opennicresolverpoolitem.h"
#include "opennicsession.h"

#if defined Q_OS_UNIX
#define DEFAULT_LOG_FILE									"/dev/tty"
#else
#define DEFAULT_LOG_FILE									"opennic.log"
#endif
#define DEFAULT_FAST_TIMER						5			/* seconds */
#define DEFAULT_refresh_timer_period			15			/* minutes */
#define DEFAULT_RESOLVER_CACHE_SIZE				3
#define DEFAULT_BOOTSTRAP_CACHE_SIZE			3
#define DEFAULT_CLIENT_TIMEOUT					3			/* seconds */
#define DEFAULT_TCP_LISTEN_PORT				    19803		/* localhost port for communication with GUI */

#define inherited QObject

OpenNICServer::OpenNICServer(QObject *parent)
: inherited(parent)
, mRefreshTimerPeriod(-1)
, mRefreshTimer(-1)
, mEnabled(true)
, mResolversInitialized(false)
{
	setRefreshTimerPeriod(DEFAULT_refresh_timer_period);
	readSettings();
	initializeServer();
	mFastTimer = startTimer(1000*DEFAULT_FAST_TIMER);
}

OpenNICServer::~OpenNICServer()
{
}

/**
  * @brief set or re-set the timer refresh period (in minutes)
  */
void OpenNICServer::setRefreshTimerPeriod(int refreshTimerPeriod)
{
	if (mRefreshTimerPeriod != refreshTimerPeriod)
	{
		mRefreshTimerPeriod = refreshTimerPeriod;
		if ( mRefreshTimer >= 0 )
		{
			killTimer(mRefreshTimer);
			mRefreshTimer=-1;
		}
		if ( mRefreshTimerPeriod >= 0 )
		{
			mRefreshTimer = startTimer((60*mRefreshTimerPeriod)*1000);
		}
	}
}

/**
  * @brief Fetch the settings.
  */
void OpenNICServer::readSettings()
{
	QSettings settings("OpenNIC", "OpenNICService");
	mTcpListenPort			= settings.value("tcp_listen_port",			DEFAULT_TCP_LISTEN_PORT).toInt();
	mLogFile				= settings.value("log_file",				DEFAULT_LOG_FILE).toString();
	mResolverCache			= settings.value("resolver_cache").toStringList();
	mRefreshTimerPeriod		= settings.value("refresh_timer_period",	DEFAULT_refresh_timer_period).toInt();
	mResolverCacheSize		= settings.value("resolver_cache_size",		DEFAULT_RESOLVER_CACHE_SIZE).toInt();

	setRefreshTimerPeriod(mRefreshTimerPeriod);
}

/**
  * @brief Store the settings.
  */
void OpenNICServer::writeSettings()
{
	QSettings settings("OpenNIC", "OpenNICService");
	settings.setValue("tcp_listen_port",			mTcpListenPort);
	settings.setValue("log_file",					mLogFile);
	settings.setValue("resolver_cache",				mResolverCache);
	settings.setValue("refresh_timer_period",		mRefreshTimerPeriod);
	settings.setValue("resolver_cache_size",		mResolverCacheSize);

	setRefreshTimerPeriod(mRefreshTimerPeriod);
}

/**
  * @brief Map server status
  * @return a map of key/value pairs
  */
QMap<QString,QVariant> OpenNICServer::mapServerStatus()
{
	QMap<QString,QVariant> map;
	map.insert("tcp_listen_port",			mTcpListenPort);
	map.insert("resolver_pool",				mResolverPool.toStringList());
	map.insert("resolver_cache",			mResolverCache);
	map.insert("refresh_timer_period",		mRefreshTimerPeriod);
	map.insert("resolver_cache_size",		mResolverCacheSize);
	map.insert("bootstrap_t1_list",			OpenNICSystem::getBootstrapT1List());
	map.insert("settings_log",				OpenNICSystem::getSystemResolverList());
	return map;
}

/**
  * @brief Map a client request key/value packet to server variables.
  */
void OpenNICServer::mapClientRequest(QMap<QString,QVariant>& map)
{
    QMapIterator<QString, QVariant>i(map);
    while (i.hasNext())
    {
        i.next();
        QString key = i.key();
        QVariant value = i.value();
        if ( key == "tcp_listen_port" )					mTcpListenPort			=	value.toInt();
        else if ( key == "log_file" )					mLogFile				=	value.toString();
        else if ( key == "resolver_cache" )				mResolverCache			=	value.toStringList();
		else if ( key == "refresh_timer_period" )
        {
			if ( value.toInt() != mRefreshTimerPeriod )
            {
                killTimer(mRefreshTimer);
                mRefreshTimer = startTimer((value.toInt()*60)*1000);
            }
			mRefreshTimerPeriod	=	value.toInt();
        }
        else if ( key == "resolver_cache_size" )
        {
            if ( value.toInt() != mResolverCacheSize ) updateDNS(value.toInt());
            mResolverCacheSize = value.toInt();
        }
        else OpenNICLog::log(OpenNICLog::Debug,"unknown key '"+key+"'");
    }
}

#if 1
/**
  * @brief Process a client request.
  * @return true if client data was received and processed else false.
  */
bool OpenNICServer::process(QTcpSocket *client)
{
	bool rc = false;
	return rc;
}
#else
/**
  * @brief Process a client request.
  * @return true if client data was received and processed else false.
  */
bool OpenNICServer::process(QTcpSocket *client)
{
	bool rc = false;
	QEventLoop loop;
	QDateTime start;
	QDateTime timeout;
	OpenNICLog::log(OpenNICLog::Debug,"process");
	QMap<QString,QVariant> clientPacket;
	QMap<QString,QVariant> serverPacket;
	QDataStream stream(client);
	start = QDateTime::currentDateTime();;
	timeout = start.addSecs(DEFAULT_CLIENT_TIMEOUT);
	while(client->isValid() && !client->bytesAvailable() && QDateTime::currentDateTime() < timeout)
	{
		loop.processEvents();
	}
	while ( client->bytesAvailable() )
	{
		clientPacket.clear();
		stream >> clientPacket;
		if ( !clientPacket.empty() )
		{
			QEventLoop loop;
			mapClientRequest(clientPacket);
			serverPacket = mapServerStatus();
			stream << serverPacket;
			client->flush();
			start = QDateTime::currentDateTime();;
			timeout = start.addSecs(DEFAULT_CLIENT_TIMEOUT);
			while(client->isValid() && client->bytesToWrite()>0 && QDateTime::currentDateTime() < timeout )
			{
				loop.processEvents();
			}
			writeSettings();					/* write changes from client */
			rc = true;
		}
		else
		{
			rc=false;
		}
	}
	return rc;
}
#endif

/**
  * @brief Get here when a task tray applet has connected.
  */
void OpenNICServer::newConnection()
{
	QTcpSocket* client;
    while ( (client = mServer.nextPendingConnection()) != NULL )
    {
        OpenNICSession* session = new OpenNICSession(client,this);
		QObject::connect(this,SIGNAL(packet(QMap<QString,QVariant>)),session,SLOT(packet(QMap<QString,QVariant>)));
		mSessions.append(session);
        session->start();
		fprintf(stderr,"session started\n");
    }
}

/**
  * @brief Set up the local server for the task tray app to attach to.
  * @return the number of resolvers
  */
int OpenNICServer::initializeServer()
{
	if (!mServer.isListening() )
	{
		QHostAddress localhost(QHostAddress::LocalHost);
		mServer.setMaxPendingConnections(10);
		if ( mServer.listen(localhost,/* mTcpListenPort */ DEFAULT_TCP_LISTEN_PORT) )
		{
			QObject::connect(&mServer,SIGNAL(newConnection()),this,SLOT(newConnection()));
			OpenNICLog::log(OpenNICLog::Information,"listening on port "+QString::number(mTcpListenPort));
		}
		else
		{
			OpenNICLog::log(OpenNICLog::Information,mServer.errorString().trimmed());
		}
	}
	return 0;
}

/**
  * @brief Perform the update function. Fetch DNS candidates, test for which to apply, and apply them.
  * @return the number of resolvers
  */
int OpenNICServer::initializeResolvers()
{
	int n, rc;
    mResolversInitialized=false;
	/** get the bootstrap resolvers... */
	QStringList bootstrapList = OpenNICSystem::getBootstrapT1List();
	for(n=0; n < bootstrapList.count(); n++)
	{
		QString resolver = bootstrapList.at(n).trimmed();
		if ( !resolver.isEmpty() )
		{
			OpenNICResolverPoolItem item(QHostAddress(resolver),"T1");
			mResolverPool.insort(item);
		}
	}
	/** apply the bootstrap resolvers */
	mResolverPool.sort();
	for(n=0; n < mResolverCacheSize && n < mResolverPool.count(); n++)
	{
		OpenNICResolverPoolItem item = mResolverPool.at(n);
		OpenNICSystem::insertSystemResolver(item.hostAddress(),n+1);
	}
	rc=n;
	/** get the T2 resolvers */
	QStringList t2List = OpenNICSystem::getBootstrapT2List();
	for(n=0; n < t2List.count(); n++)
	{
		QString resolver = t2List.at(n).trimmed();
		if ( !resolver.isEmpty() )
		{
			OpenNICResolverPoolItem item(QHostAddress(resolver),"T2");
			mResolverPool.insort(item);
            mResolversInitialized=true;
		}
	}
	return rc;
}

/**
  * @brief Perform the update function. Fetch DNS candidates, test for which to apply, and apply them.
  * @return the number of resolvers
  */
int OpenNICServer::updateDNS(int resolverCount)
{
	int n;
	mResolverCache.clear();
	mResolverPool.sort();
	for(n=0; n < mResolverPool.count() && n < resolverCount; n++)
	{
		OpenNICResolverPoolItem item = mResolverPool.at(n);
		OpenNICSystem::insertSystemResolver(item.hostAddress(),n+1);
		mResolverCache.append(item.hostAddress().toString());
	}
	return n;
}

QString OpenNICServer::copyright()
{
	return "OpenNICServer V"+QString(VERSION_STRING)+ tr( " (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>" );
}

QString OpenNICServer::license()
{
	return QString( copyright() +
						tr( "\"THE BEER-WARE LICENSE\" (Revision 42):\n"
						"Mike Sharkey wrote this thing. As long as you retain this notice you\n"
						"can do whatever you want with this stuff. If we meet some day, and you think\n"
						"this stuff is worth it, you can buy me a beer in return.\n" )
					   );
}

/**
  * @brief purge dead (closed) sessions.
  */
void OpenNICServer::purgeDeadSesssions()
{
	/* get here regularly, purge dead sessions... */
	for(int n=0; n < mSessions.count(); n++)
	{
		OpenNICSession* session = mSessions.at(n);
		if ( session->isFinished() )
		{
			fprintf(stderr,"session disposed\n");
			mSessions.takeAt(n);
			delete session;
		}
	}
}

/**
  * @brief announce packets to sessions
  */
void OpenNICServer::announcePackets()
{
	if ( mSessions.count() )
	{
		emit packet(mapServerStatus());
	}
}

/**
  * @brief get here to initiate cold bootstrap
  */
void OpenNICServer::coldBoot()
{
	/* get here just once just after startup */
	readSettings();
	initializeResolvers();
	if ( mResolversInitialized )
	{
		initializeServer();
	}
}

/**
  * @brief test the integrity of the resolver cache. look for dead resolvers.
  */
bool OpenNICServer::testResolverCache()
{
	for(int n=0; n < mResolverCache.count(); n++ )
	{
		OpenNICResolverPoolItem item = mResolverCache.at(n);
		if ( !item.alive() )
		{
			return false;
		}
	}
	return true;
}

/**
  * @brief get here once in a while to see if we need to refresh any resolvers.
  */
void OpenNICServer::refreshResolvers(bool force)
{
	readSettings();
	if (force || mResolverCache.size()==0 || mResolverCache.size() < mResolverCacheSize )
	{
		updateDNS(mResolverCacheSize);
	}
	if (mResolverCache.count() && !testResolverCache());	/* how are our currently active resolvers doing?.. */
	{
		updateDNS(mResolverCacheSize);						/* ...not so good, get new ones. */
	}
}

/**
  * @brief get here on timed events.
  */
void OpenNICServer::timerEvent(QTimerEvent* e)
{
	if ( e->timerId() == mFastTimer )
	{
		purgeDeadSessions();									/* free up closed gui sessions */
		refreshResolvers();										/* try to be smart */
		announcePackets();										/* tell gui sessions what they need to know */
	}
	else if ( e->timerId() == mRefreshTimer )					/* get here once in a while, a slow timer... */
	{
		if ( !mResolversInitialized )							/* have we started any resolvers yet? */
		{
			coldBoot();											/* start from scratch */
		}
		else
		{
			refreshResolvers(true);								/* force a resolver cache refresh */
		}
	}
	else
	{
		inherited::timerEvent(e);
	}
}




