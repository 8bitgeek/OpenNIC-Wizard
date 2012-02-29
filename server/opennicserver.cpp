/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicserver.h"
#include "opennicsystem.h"
#include "opennicresolverpoolitem.h"
#include "opennicsession.h"

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

#define DEFAULT_FAST_TIMER						30			/* seconds */
#define DEFAULT_REFRESH_TIMER_PERIOD			15			/* minutes */
#define DEFAULT_RESOLVER_CACHE_SIZE				3
#define DEFAULT_BOOTSTRAP_CACHE_SIZE			3
#define DEFAULT_CLIENT_TIMEOUT					3			/* seconds */
#define DEFAULT_TCP_LISTEN_PORT				    19803		/* localhost port for communication with GUI */
#define MAX_LOG_LINES							100			/* max lines to keep in log cache */

#define inherited QObject

OpenNICServer::OpenNICServer(QObject *parent)
: inherited(parent)
, mRefreshTimerPeriod(-1)
, mRefreshTimer(-1)
, mEnabled(true)
, mResolversInitialized(false)
{
	setRefreshTimerPeriod(DEFAULT_REFRESH_TIMER_PERIOD);
	readSettings();
	initializeServer();
	mFastTimer = startTimer(1000*DEFAULT_FAST_TIMER);
}

OpenNICServer::~OpenNICServer()
{
}

/**
  * @brief log a message
  */
void OpenNICServer::log(QString msg)
{
	mLog << QDateTime::currentDateTime().toString("yyMMddhhmmss")+"|"+msg;
}

/**
  * @brief purge the log
  */
void OpenNICServer::logPurge()
{
	mLog.clear();
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
	mRefreshTimerPeriod		= settings.value("refresh_timer_period",	DEFAULT_REFRESH_TIMER_PERIOD).toInt();
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
	settings.setValue("refresh_timer_period",		mRefreshTimerPeriod);
	settings.setValue("resolver_cache_size",		mResolverCacheSize);

	setRefreshTimerPeriod(mRefreshTimerPeriod);
}

/**
  * @brief Make a server packet
  * @return a map of key/value pairs
  */
QMap<QString,QVariant> OpenNICServer::makeServerPacket()
{
	QMap<QString,QVariant> map;
	map.insert("tcp_listen_port",			mTcpListenPort);
	map.insert("resolver_pool",				mResolverPool.toStringList());
	map.insert("resolver_cache",			mResolverCache.toStringList());
	map.insert("refresh_timer_period",		mRefreshTimerPeriod);
	map.insert("resolver_cache_size",		mResolverCacheSize);
	map.insert("bootstrap_t1_list",			OpenNICSystem::getBootstrapT1List());
	map.insert("system_text",				OpenNICSystem::getSystemResolverList());
	map.insert("journal_text",				mLog);
	return map;
}

/**
  * @brief Map a client  key/value packet to server variables.
  */
void OpenNICServer::sessionPacket(OpenNICSession* session, QMap<QString,QVariant> map)
{
	QMapIterator<QString, QVariant>i(map);
	while (i.hasNext())
	{
		i.next();
		QString key = i.key();
		QVariant value = i.value();
		if ( key == "tcp_listen_port" )					mTcpListenPort			=	value.toInt();
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
		else if ( key == "initialize" )
		{
			log("client version "+value.toString());
		}
		else log("unhandled key from client '"+key+"'");
	}
}

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
		QObject::connect(session,SIGNAL(sessionPacket(OpenNICSession*,QMap<QString,QVariant>)),this,SLOT(sessionPacket(OpenNICSession*,QMap<QString,QVariant>)));
		mSessions.append(session);
        session->start();
		log(tr("** client session created **"));
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
			log(tr("listening on port ")+QString::number(mTcpListenPort));
		}
		else
		{
			fprintf(stderr,"%s\n",mServer.errorString().trimmed().toAscii().data());
		}
	}
	return 0;
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
			log("** client session disposed **");
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
		emit packet(makeServerPacket());
		logPurge();
	}
}

/**
  * @brief get here to initiate cold bootstrap
  */
void OpenNICServer::coldBoot()
{
	log("** COLD BOOT **");
	log(copyright());
	readSettings();
	bootstrapResolvers();
	if ( mResolversInitialized )
	{
		initializeServer();
	}
}

/**
  * @brief Perform the update function. Fetch DNS candidates, test for which to apply, and apply them.
  * @return the number of resolvers
  */
int OpenNICServer::bootstrapResolvers()
{
	int n, rc;
	mResolversInitialized=false;
	/** get the bootstrap resolvers... */
	mResolverPool.clear();
	QStringList bootstrapList = OpenNICSystem::getBootstrapT1List();
	log(tr("Found ")+QString::number(bootstrapList.count())+tr(" T1 resolvers"));
	for(n=0; n < bootstrapList.count(); n++)
	{
		QString resolver = bootstrapList.at(n).trimmed();
		if ( !resolver.isEmpty() )
		{
			OpenNICResolverPoolItem item(QHostAddress(resolver),"T1");
			mResolverPool.insort(item);
			log(item.toString());
		}
	}
	/** apply the bootstrap resolvers */
	log(tr("Randomizing T1 list..."));
	mResolverPool.randomize();
	int nBootstrapResolvers = mResolverCacheSize <= mResolverPool.count() ? mResolverCacheSize : mResolverPool.count();
	log(tr("Applying ")+QString::number(nBootstrapResolvers)+tr(" T1 resolvers..."));
	for(n=0; n < nBootstrapResolvers; n++)
	{
		OpenNICResolverPoolItem item = mResolverPool.at(n);
		OpenNICSystem::insertSystemResolver(item.hostAddress(),n+1);
		log(" > "+item.toString());
	}
	rc=n;
	/** get the T2 resolvers */
	log(tr("Fetching T2 resolvers..."));
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
	log(tr("Found ")+QString::number(t2List.count())+tr(" T2 resolvers"));
	log("mResolversInitialized="+QString(mResolversInitialized?"TRUE":"FALSE"));
	return rc;
}

/**
  * @brief Perform the update function. Fetch DNS candidates, test for which to apply, and apply them.
  * @return the number of resolvers
  */
int OpenNICServer::updateDNS(int resolverCount)
{
	bool replaceWithProposed = false;
	OpenNICResolverPool proposed;
	mResolverPool.sort();
	for(int n=0; n < mResolverPool.count() && n < resolverCount; n++)
	{
		OpenNICResolverPoolItem item = mResolverPool.at(n);
		proposed.append(item);
	}
	/** see if what we are proposing is different than what we have cach'ed already... */
	for(int n=0; n < proposed.count(); n++)
	{
		OpenNICResolverPoolItem item = proposed.at(n);
		if (!mResolverCache.contains(item))
		{
			replaceWithProposed = true;
		}
	}
	if ( replaceWithProposed )
	{
		int n;
		mResolverCache.clear();
		proposed.sort();
		log("applying new resolver cache ("+QString::number(proposed.count())+") items...");
		for(n=0; n < proposed.count(); n++)
		{
			OpenNICResolverPoolItem item = proposed.at(n);
			OpenNICSystem::insertSystemResolver(item.hostAddress(),n+1);
			mResolverCache.append(item);
			log(" > "+item.toString());
		}
		log(tr("applied ")+QString::number(n)+tr(" T2 resolvers"));
		return n;
	}
	return 0;
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
			log( tr("** DETECTED ACTIVE RESOLVER FAILURE '")+item.hostAddress().toString()+"' **");
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
	if ( !mResolversInitialized )							/* have we started any resolvers yet? */
	{
		coldBoot();											/* start from scratch */
	}
	if (force || mResolverCache.count()==0 || mResolverCache.count() < mResolverCacheSize )
	{
		updateDNS(mResolverCacheSize);
	}
	if (mResolverCache.count() && !testResolverCache());	/* how are our currently active resolvers doing?.. */
	{
		updateDNS(mResolverCacheSize);						/* ...not so good, get new ones. */
	}
}

/**
  * @brief prune the log 'file'
  */
void OpenNICServer::pruneLog()
{
	while (mLog.count() > MAX_LOG_LINES)
	{
		mLog.takeFirst();
	}
}

/**
  * run regular functions
  */
void OpenNICServer::runOnce()
{
	purgeDeadSesssions();									/* free up closed gui sessions */
	refreshResolvers();										/* try to be smart */
	announcePackets();										/* tell gui sessions what they need to know */
	pruneLog();												/* don't let the log get out of hand */
}

/**
  * @brief get here on timed events.
  */
void OpenNICServer::timerEvent(QTimerEvent* e)
{
	if ( e->timerId() == mFastTimer )
	{
		runOnce();											/* don't let the log get out of hand */
	}
	else if ( e->timerId() == mRefreshTimer )				/* get here once in a while, a slow timer... */
	{
		refreshResolvers(true);								/* force a resolver cache refresh */
	}
	else
	{
		inherited::timerEvent(e);
	}
}




