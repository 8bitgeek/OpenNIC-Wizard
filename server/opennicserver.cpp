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

#define DEFAULT_FAST_TIMER						5			/* seconds */
#define DEFAULT_REFRESH_TIMER_PERIOD			1			/* minutes */
#define DEFAULT_RESOLVER_CACHE_SIZE				3
#define DEFAULT_BOOTSTRAP_CACHE_SIZE			3
#define DEFAULT_CLIENT_TIMEOUT					3			/* seconds */
#define DEFAULT_TCP_LISTEN_PORT				    19803		/* localhost port for communication with GUI */
#define MAX_LOG_LINES							100			/* max lines to keep in log cache */

#define inherited QObject

QMutex					OpenNICServer::mLocker;
QMutex					OpenNICServer::mPacketLocker;
int						OpenNICServer::mRefreshTimerPeriod=0;
int						OpenNICServer::mPreviousRefreshTimerPeriod=0;
int						OpenNICServer::mResolverCacheSize=0;
QMap<QString,QVariant>	OpenNICServer::mServerPacket;				/** the next packet to go out to the gui */

OpenNICServer::OpenNICServer(QObject *parent)
: inherited(parent)
, mRefreshTimer(-1)
, mEnabled(true)
, mResolversInitialized(false)
{
	setRefreshPeriod(DEFAULT_REFRESH_TIMER_PERIOD);
	readSettings();
	initializeServer();
	mFastTimer = startTimer(1000*DEFAULT_FAST_TIMER);
}

OpenNICServer::~OpenNICServer()
{
}

int OpenNICServer::refreshPeriod()
{
	int rc;
	mLocker.lock();
	rc = mRefreshTimerPeriod;
	mLocker.unlock();
	return rc;
}

int OpenNICServer::resolverCacheSize()
{
	int rc;
	mLocker.lock();
	rc = mResolverCacheSize;
	mLocker.unlock();
	return rc;
}

void OpenNICServer::setRefreshPeriod(int period)
{
	mLocker.lock();
	mRefreshTimerPeriod = period;
	mLocker.unlock();
}

void OpenNICServer::setResolverCacheSize(int size)
{
	mLocker.lock();
	mResolverCacheSize = size;
	mLocker.unlock();
}

/**
  * @brief log a message
  */
void OpenNICServer::log(QString msg)
{
	QString str = QDateTime::currentDateTime().toString("yyMMddhhmmss")+"|"+msg;
	mLog << str;
	fprintf(stderr,"%s\n",str.toAscii().data());
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
  * @brief purge the log
  */
void OpenNICServer::logPurge()
{
	mLog.clear();
}

/**
  * @brief Fetch the settings.
  */
void OpenNICServer::readSettings()
{
	QSettings settings("OpenNIC", "OpenNICService");
	mTcpListenPort			= settings.value("tcp_listen_port",			DEFAULT_TCP_LISTEN_PORT).toInt();
	setRefreshPeriod(settings.value("refresh_timer_period",	DEFAULT_REFRESH_TIMER_PERIOD).toInt());
	setResolverCacheSize(settings.value("resolver_cache_size",DEFAULT_RESOLVER_CACHE_SIZE).toInt());
}

/**
  * @brief Store the settings.
  */
void OpenNICServer::writeSettings()
{
	QSettings settings("OpenNIC", "OpenNICService");
	settings.setValue("tcp_listen_port",			mTcpListenPort);
	settings.setValue("refresh_timer_period",		refreshPeriod());
	settings.setValue("resolver_cache_size",		resolverCacheSize());
}

/**
  * @brief Make a server packet
  * @return a map of key/value pairs
  */
QMap<QString,QVariant>& OpenNICServer::makeServerPacket(QMap<QString,QVariant>& packet)
{
	packet.insert("tcp_listen_port",			mTcpListenPort);
	packet.insert("resolver_pool",				mResolverPool.toStringList());
	packet.insert("resolver_cache",			mResolverCache.toStringList());
	packet.insert("refresh_timer_period",		refreshPeriod());
	packet.insert("resolver_cache_size",		resolverCacheSize());
	packet.insert("bootstrap_t1_list",			OpenNICSystem::getBootstrapT1List());
	packet.insert("system_text",				OpenNICSystem::getSystemResolverList());
	packet.insert("journal_text",				mLog);
	return packet;
}

/**
  * @brief Get here when a task tray applet has connected.
  */
void OpenNICServer::newConnection()
{
	QTcpSocket* client;
	while ( (client = mServer.nextPendingConnection()) != NULL )
	{
		OpenNICSession* session = new OpenNICSession(client);
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
			log("** CLIENT SESSION DISPOSED **");
			mSessions.takeAt(n);
			delete session;
		}
	}
}

/**
  * @brief from separate thread, call this method to get the server packet
  */
void OpenNICServer::getPacket(QMap<QString,QVariant>& packet)
{
	mPacketLocker.lock();
	packet = mServerPacket;
	mPacketLocker.unlock();
}

/**
  * @brief announce packets to sessions
  */
void OpenNICServer::announcePackets()
{
	mPacketLocker.lock();
	makeServerPacket(mServerPacket);
	mPacketLocker.unlock();
	logPurge();
}

/**
  * @brief get here to initiate cold bootstrap
  */
void OpenNICServer::coldBoot()
{
	log("** COLD BOOT **");
	log(copyright());
	log(license());
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
	int nBootstrapResolvers = resolverCacheSize() <= mResolverPool.count() ? resolverCacheSize() : mResolverPool.count();
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
	log("** UPDATE DNS **");
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
		log("Applying new resolver cache of ("+QString::number(proposed.count())+") items...");
		for(n=0; n < proposed.count(); n++)
		{
			OpenNICResolverPoolItem item = proposed.at(n);
			OpenNICSystem::insertSystemResolver(item.hostAddress(),n+1);
			mResolverCache.append(item);
			log(" > "+item.toString());
		}
		log(tr("Applied ")+QString::number(n)+tr(" T2 resolvers"));
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
			log("** ACTIVE RESOLVER "+item.hostAddress().toString()+"' NOT RESPONDING **");
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
	if (force || mResolverCache.count()==0 || mResolverCache.count() < resolverCacheSize() )
	{
		updateDNS(resolverCacheSize());
	}
	if (mResolverCache.count() && !testResolverCache())		/* how are our currently active resolvers doing?.. */
	{
		updateDNS(resolverCacheSize());						/* ...not so good, get new ones. */
	}
}

/**
  * @brief in the event the refresh perid has changed update the timer
  */
void OpenNICServer::updateRefreshTimerPeriod()
{
	if ( refreshPeriod() != mPreviousRefreshTimerPeriod )
	{
		if (mRefreshTimer >= 0)
		{
			killTimer(mRefreshTimer);
			mRefreshTimer=-1;
		}
		mRefreshTimer = startTimer((60*refreshPeriod())*1000);
	}
}

/**
  * run regular functions
  */
void OpenNICServer::runOnce()
{
	updateRefreshTimerPeriod();								/* in case refresh period has changed */
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
		log("** REFRESH TIMER **");
		refreshResolvers(true);								/* force a resolver cache refresh */
	}
	else
	{
		inherited::timerEvent(e);
	}
}




