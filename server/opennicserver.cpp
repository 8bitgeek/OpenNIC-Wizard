/*
 * This file is a part of OpenNIC Wizard
 * Copywrong (c) 2012-2022 Mike Sharkey
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 1776):
 * <mike@8bitgeek.net> wrote this file.
 * As long as you retain this notice you can do whatever you want with this
 * stuff. If we meet some day, and you think this stuff is worth it,
 * you can buy me a beer in return. ~ Mike Sharkey
 * ----------------------------------------------------------------------------
 */
#include "opennicserver.h"
#include "opennicresolver.h"

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
#define BOOTSTRAP_TIMER							(5*1000)	/* boostrap timer interva */
#define BOOTSTRAP_TICKS							24			/* number of bootstrap ticks */


#define DEFAULT_SCORE_RULES	"var score = maxPoolLatency - resolverAverageLatency;\n" \
                            "if (resolverKind == \"T1\")    score *= 0.6666;\n" \
                            "if (resolverKind == \"T2\")    score *= 1.5;\n" \
                            "if (resolverStatus == \"R\")   score *= 0.5;\n" \
                            "if (resolverStatus == \"Y\")   score *= 0.75;\n" \
							"if (resolvesNIC == true)       score *= 1.75;\n" \
                            "score;\n"

extern int GlobalShutdown;

QStringList		OpenNICServer::mLog;					/* the log text */
QString			OpenNICServer::mScoreRules;				/* the score rules javascript text */
bool			OpenNICServer::mScoreInternal=true;		/* use internal scoring rules? */

#define inherited QObject

OpenNICServer::OpenNICServer(QObject *parent)
: inherited(parent)
, mRefreshTimerPeriod(0)
, mResolverCacheSize(0)
, mEnabled(true)
, mRefreshTimer(-1)
, mFastTimer(-1)
, mSeconds(0)
, mResolversInitialized(false)
, mTcpListenPort(DEFAULT_TCP_LISTEN_PORT)
, mUpdatingDNS(false)
, mBootstrapTicks(0)
, mInColdBoot(false)
{
	readSettings();
#if defined(Q_OS_UNIX)
	mSystem = new OpenNICSystem_Linux(mEnabled,mNetworkInterface);
#elif defined(Q_OS_WIN32)
	mSystem = new OpenNICSystem_Win(mEnabled,mNetworkInterface);
#else 
	#error System Not Defined
#endif
	mSystem->startup();
	initializeServer();
	mFastTimer = startTimer(1000*DEFAULT_FAST_TIMER);
	mBootstrapTimer = startTimer(BOOTSTRAP_TIMER);
	mSecondTimer = startTimer(1000);
}

OpenNICServer::~OpenNICServer()
{
	delete mSystem;
}

int OpenNICServer::refreshPeriod()
{
	return mRefreshTimerPeriod;
}

/**
  * @brief Set refresh period ni minutes
  */
void OpenNICServer::setRefreshPeriod(int period)
{
	if ( mRefreshTimerPeriod != period && period >= 0 )
	{
		mRefreshTimerPeriod = period;
		if ( mRefreshTimer >= 0 )
		{
			killTimer(mRefreshTimer);
			mRefreshTimer=-1;
		}
		log(tr("** DNS REFRESH IN ")+QString::number(mRefreshTimerPeriod)+tr(" MINUTES **"));
		mRefreshTimer = startTimer((mRefreshTimerPeriod*60)*1000);
	}
}

int OpenNICServer::resolverCacheSize()
{
	return mResolverCacheSize;
}

/**
  * @brief set the resolver cache size and update the dns with the settings
  */
void OpenNICServer::setResolverCacheSize(int size)
{
	if ( mResolverCacheSize != size && size >= 0 )
	{
		mResolverCacheSize = size;
		log(tr("** ACTIVE CACHE SET TO ")+QString::number(mResolverCacheSize)+tr(" RESOLVERS **"));
		updateDNS(mResolverCacheSize);
	}
}

/**
  * @brief log a message
  */
void OpenNICServer::log(QString msg)
{
	QString str = QDateTime::currentDateTime().toString("yyMMddhhmmss")+"|"+msg;
	mLog << str;
    printf("%s\n",str.toLocal8Bit().data());
	while(mLog.count() > MAX_LOG_LINES)
	{
		mLog.takeAt(0);
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
	QSettings settings(QSettings::IniFormat, QSettings::SystemScope, "OpenNIC", "OpenNICService");
	// log(tr("<< ")+settings.fileName());
	mTcpListenPort				= settings.value(	"tcp_listen_port",			DEFAULT_TCP_LISTEN_PORT).toInt();
	setRefreshPeriod(			settings.value(		"refresh_timer_period",		DEFAULT_REFRESH_TIMER_PERIOD).toInt());
	setResolverCacheSize(		settings.value(		"resolver_cache_size",		DEFAULT_RESOLVER_CACHE_SIZE).toInt());
	mScoreRules					= settings.value(	"score_rules",				DEFAULT_SCORE_RULES).toString();
	mScoreInternal				= settings.value(	"score_internal",			true).toBool();
	mEnabled = 					settings.value( 	"enabled",					true ).toBool();
	mNetworkInterface = 		settings.value( 	"network_interface", 		"" ).toString();
	if ( mSystem )
	{
		mSystem->setEnabled( mEnabled );
		mSystem->setInterfaceName( mNetworkInterface );
	}
}

/**
  * @brief Store the settings.
  */
void OpenNICServer::writeSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::SystemScope, "OpenNIC", "OpenNICService");
	log(tr(">> ")+settings.fileName());
	settings.setValue("tcp_listen_port",			mTcpListenPort);
	settings.setValue("refresh_timer_period",		refreshPeriod());
	settings.setValue("resolver_cache_size",		resolverCacheSize());
	settings.setValue("score_rules",				mScoreRules);
	settings.setValue("score_internal",				mScoreInternal);
	settings.setValue("enabled",					mSystem->enabled());
	settings.setValue("network_interface",			mSystem->interfaceName());
	settings.sync();
	if ( settings.status() != QSettings::NoError )
	{
		QString reason;
		switch( settings.status() )
		{
			case QSettings::AccessError: reason="An access error occurred (e.g. trying to write to a read-only file)"; break;
			case QSettings::FormatError: reason="A format error occurred (e.g. loading a malformed INI file)"; break;
			default: reason="Unknown failure"; break;
		}
		log(tr("** SETTINGS ")+settings.fileName()+QString(" -")+reason);
	}
}

/**
  * @brief Get here when a task tray applet has connected.
  */
void OpenNICServer::newConnection()
{
	QTcpSocket* socket;
	while ( (socket = mServer.nextPendingConnection()) != NULL )
	{
		OpenNICNet* net = new OpenNICNet(socket);
		QObject::connect(net,SIGNAL(dataReady(OpenNICNet*)),this,SLOT(dataReady(OpenNICNet*)),Qt::DirectConnection);
		QObject::connect(net,SIGNAL(pollKeyValue(QString&,QVariant&,bool&)),this,SLOT(pollKeyValue(QString&,QVariant&,bool&)),Qt::DirectConnection);
		mSessions.append(net);
		log(tr("** client session created **"));
	}
}

/**
  * @brief The other end is asking for data.
  */
void OpenNICServer::pollKeyValue(QString& key, QVariant& value, bool& valid)
{
	valid = true;
	if (key == OpenNICPacket::tcp_listen_port)				value = mTcpListenPort;
	else if (key == OpenNICPacket::refresh_timer_period)	value = refreshPeriod();
	else if (key == OpenNICPacket::resolver_cache_size)		value = resolverCacheSize();
	else if (key == OpenNICPacket::resolver_pool)			value = mResolverPool.toStringList();
	else if (key == OpenNICPacket::resolver_cache)			value = mResolverCache.toStringList();
    else if (key == OpenNICPacket::bootstrap_t1_list)		value = OpenNICSystem::instance()->getBootstrapT1List();
    else if (key == OpenNICPacket::bootstrap_domains)		value = OpenNICSystem::instance()->getTestDomains().toStringList();
    else if (key == OpenNICPacket::system_text)				value = OpenNICSystem::instance()->getSystemResolverList();
	else if (key == OpenNICPacket::journal_text)			value = mLog;
	else if (key == OpenNICPacket::async_message)			value = mAsyncMessage;
	else if (key == OpenNICPacket::score_rules)				value = mScoreRules;
	else if (key == OpenNICPacket::score_internal)			value = mScoreInternal;
	else if (key == OpenNICPacket::opennic_enabled)			value = mSystem->enabled();
	else if (key == OpenNICPacket::interface_list)			value = mSystem->interfaceNames();
	else if (key == OpenNICPacket::interface)				value = mSystem->interfaceName();
	else valid = false;
}

/**
  * @brief get here on data available from client
  */
void OpenNICServer::dataReady(OpenNICNet* net)
{
	if ( net->isLive() )
	{
		QMapIterator<QString, QVariant>i(net->rxPacket().data());
		while (i.hasNext())
		{
			i.next();
			QString key = i.key();
			QVariant value = i.value();
			if ( key == OpenNICPacket::resolver_cache_size )
			{
				if (resolverCacheSize() != value.toInt())
				{
					mAsyncMessage = tr("Settings Applied");
				}
				setResolverCacheSize(value.toInt());
			}
			else if ( key == OpenNICPacket::refresh_timer_period )
			{
				if (refreshPeriod() != value.toInt())
				{
					mAsyncMessage = tr("Settings Applied");
				}
				setRefreshPeriod(value.toInt());
			}
			else if ( key == OpenNICPacket::bootstrap_t1_list )
			{
                if ( OpenNICSystem::instance()->saveBootstrapT1List(value.toStringList()) )
				{
					mAsyncMessage = tr("Bootstrap T1 List Saved");
				}
				else
				{
					mAsyncMessage = tr("There was a problem saving the T1 bootstrap list");
				}
			}
			else if ( key == OpenNICPacket::bootstrap_domains )
			{
                if ( OpenNICSystem::instance()->saveTestDomains(value.toStringList()) )
				{
					mAsyncMessage = tr("Domain List Saved");
				}
				else
				{
					mAsyncMessage = tr("There was a problem saving the domains list");
				}
			}
			else if ( key == OpenNICPacket::update_dns )
			{
				updateDNS(resolverCacheSize());
			}
			else if ( key == OpenNICPacket::score_rules )
			{
				mScoreRules = value.toString();
			}
			else if ( key == OpenNICPacket::score_internal )
			{
				mScoreInternal = value.toBool();
			}
			else if (key == OpenNICPacket::resolver_history)
			{
				QString address = value.toString();
				net->txPacket().set(OpenNICPacket::resolver_history,mResolverPool.toStringList(address));
				net->send(true);
			}
			else if (key == OpenNICPacket::interface)
			{
				mSystem->setInterfaceName(value.toString());
			}
			else if (key == OpenNICPacket::opennic_enabled)
			{
				if ( value.toBool() != mSystem->enabled() )
					mAsyncMessage = mSystem->enabled() ? tr("OpenNIC Disabled") :  tr("OpenNIC Enabled");
				mSystem->setEnabled(value.toBool());
			}
		}
	}
	writeSettings();
	if (!mAsyncMessage.isEmpty())
	{
		net->txPacket().set(OpenNICPacket::async_message,mAsyncMessage);
		net->send(true);
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
		QHostAddress localhost("127.0.0.1");
		mServer.setMaxPendingConnections(10);
		if ( mServer.listen(localhost, DEFAULT_TCP_LISTEN_PORT) )
		{
			QObject::connect(&mServer,SIGNAL(newConnection()),this,SLOT(newConnection()));
			log(tr("listening on port ")+QString::number(mTcpListenPort));
		}
		else
		{
            fprintf(stderr,"%s\n",mServer.errorString().trimmed().toLocal8Bit().data());
		}
	}
	return 0;
}

QString OpenNICServer::copyright()
{
    return "OpenNICServer V"+QString(VERSION_STRING)+ tr( " Copywrong (c) 2012-2022 Mike Sharkey <mike@8bitgeek.net>" );
}

QString OpenNICServer::license()
{
    return QString( 
						tr( "----------------------------------------------------------------------------\n"
							"\"THE BEER-WARE LICENSE\" (Revision 1776):\n"
							"<mike@8bitgeek.net> wrote this file.\n"
							"As long as you retain this notice you can do whatever you want with this stuff.\n"
							"If we meet some day, and you think this stuff is worth it,\n"
							"you can buy me a beer in return. ~ Mike Sharkey\n"
							"----------------------------------------------------------------------------\n"
                        )
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
		OpenNICNet* net = mSessions[n];
		if ( !net->isLive() )
		{
			log(tr("** CLIENT SESSION DISPOSED **"));
			mSessions.takeAt(n);
			delete net;
		}
	}
}

/**
  * @brief get here to initiate cold bootstrap
  */
void OpenNICServer::coldBoot()
{
	if ( !mInColdBoot )
	{
		mInColdBoot=true;
		log(tr("** COLD BOOT **"));
		log(copyright());
		log(license());
		readSettings();
		while(!mResolversInitialized)
		{
			bootstrapResolvers();
			if ( mResolversInitialized )
			{
				initializeServer();
			}
			else
			{
				log(tr("** COLD BOOT FAILED - RETRY IN 5 SECONDS **"));
				delay(5);
			}
		}
		mInColdBoot=false;
	}
}

/**
  * @brief Perform the update function. Fetch DNS candidates, test for which to apply, and apply them.
  * @return the number of resolvers
  */
int OpenNICServer::bootstrapResolvers()
{
	mResolversInitialized=false;
	
	/** Get the bootstrap resolvers... */
    QStringList bootstrapList = OpenNICSystem::instance()->getBootstrapT1List();
	OpenNICResolverPool proposed;
	mResolverPool.clear();
	mResolverPool.fromIPList(bootstrapList,"T1");
	mResolverPool.randomize();
	log(tr("Found ")+QString::number(mResolverPool.count())+tr(" T1 resolvers"));
	
	if (mResolverPool.count() < resolverCacheSize())
	{
		log(tr("** Warning: T1 bootstrap resolver count is less than resolver cache size"));
	}
	
	for(int n=0; n < ((resolverCacheSize() <= mResolverPool.count()) ? resolverCacheSize() : mResolverPool.count()); n++)
	{
		OpenNICResolver* resolver = mResolverPool.at(n);
		proposed.append(resolver);
	}
	
	/** Apply the T1 bootstrap resolvers */
	log(tr("Randomizing T1 list"));
	proposed.randomize();
	log(tr("Applying (")+QString::number(proposed.count())+tr(") resolvers from the T1 list"));
	replaceActiveResolvers(proposed);
	
	/** Get the T2 resolvers */
	log(tr("Fetching T2 resolvers"));
    bootstrapList.clear();
	for(int tries=0; bootstrapList.isEmpty() && tries < 5; tries++)
    {
        bootstrapList = OpenNICSystem::instance()->getBootstrapT2List();
    }

	/** Start with a randomized list */
	mResolverPool.fromIPList(bootstrapList,"T2");
	log(tr("Randomizing T2 Resolvers"));
	mResolverPool.randomize();
	proposed.clear();
	
	for(int n=0; n < ((resolverCacheSize() <= mResolverPool.count()) ? resolverCacheSize() : mResolverPool.count()); n++)
	{
		OpenNICResolver* resolver = mResolverPool.at(n);
		proposed.append(resolver);
	}
	
	if (proposed.count())
	{
		log(tr("Applying (")+QString::number(proposed.count())+tr(") resolvers from the T1 list"));
		mResolversInitialized=replaceActiveResolvers(proposed);
	}
	else
	{
		log(tr("** Critical: no T2 resolvers found **"));
	}

	if ( mResolversInitialized )
	{
		log(tr("** RESOLVERS INITIALIZED **"));
	}
	else 
	{
		log(tr("** RESOLVERS NOT INITIALIZED **"));
	}

	return mResolversInitialized ? proposed.count() : 0;
}

/**
  * @brief log the resolver pool text
  */
void OpenNICServer::logResolverPool(QString msg, OpenNICResolverPool &pool)
{
	log(msg);
	for(int n=0; n < pool.count(); n++)
	{
		OpenNICResolver* resolver = pool.at(n);
		log(resolver->toString());
	}
}

/**
  * @brief determine if the active resolvers should be replaces with those proposed.
  * @return true to replace active resolvers with those proposed, else false
  */
bool OpenNICServer::shouldReplaceWithProposed(OpenNICResolverPool& proposed)
{
	logResolverPool("**PROPOSED**",proposed);
	logResolverPool("**ACTIVE**",mResolverCache);
	if ( proposed.count() >= 2 && proposed.count() == mResolverCache.count() )
	{
		int diffCount=0; /* number of differences */
		for(int n=0; n < proposed.count(); n++)
		{
			OpenNICResolver* resolver = proposed.at(n);
			if (!mResolverCache.contains(resolver))
			{
				++diffCount;
			}
		}
		if ( diffCount >= mResolverCache.count()/2 )
		{
			log(tr("Recommending active resolvers be replaced due to >= %50 change proposed"));
			return true;
		}
		else
		{
			log(tr("Not recommending active resolvers be replaced due to < %50 change proposed"));
			return false;
		}
	}
	else if (proposed.count() == 1 && mResolverCache.count() == 1)
	{
		if( proposed.at(0) == mResolverCache.at(0) )
		{
			log(tr("Recommending active resolver be replaced due single proposed <> single active resolver"));
			return true;
		}
		else
		{
			log(tr("Not recommending active resolvers be replaced due tosingle proposed == single active resolver"));
			return false;
		}
	}
	log(tr("Recommending resolvers be replaced with proposed as a default measure"));
	return true; /* when in doubt, replace */
}

/**
  * @brief replace the active resolvers with those proposed
  */
bool OpenNICServer::replaceActiveResolvers(OpenNICResolverPool& proposed)
{
	bool applied=false;
	mResolverCache.clear();
	proposed.sort();
	log("Begin applying updated resolver cache of ("+QString::number(proposed.count())+") items...");
    if ( (applied=OpenNICSystem::instance()->beginUpdateResolvers()) )
	{
		for(int index=0; index < proposed.count(); index++)
		{
			OpenNICResolver* resolver = proposed.at(index);
            if ( (applied=OpenNICSystem::instance()->updateResolver(resolver->hostAddress())) )
			{
				log(" > "+resolver->toString());
				mResolverCache.append(resolver);
			}
			log(tr("** EXIT ")+(applied?tr("OK"):tr("FAIL")));
		}
        if ( !(applied=OpenNICSystem::instance()->endUpdateResolvers()) )
		{
			log("** Operating system failed to commit resolver cache changes **");
		}
	}
	else
	{
		log(tr("** Operating system refused to begin comitting changes **"));
	}
	if (applied)
	{
		log(tr("Next scheduled refresh proposal in ")+QString::number(refreshPeriod())+tr(" minutes"));
	}
	return applied;
}

/**
  * @brief Perform the update function. Fetch DNS candidates, test for which to apply, and apply them.
  * @return the number of resolvers
  */
int OpenNICServer::updateDNS(int resolverCount)
{
	int rc=0;
	if ( !mUpdatingDNS )
	{
		mUpdatingDNS=true;
		log(tr("** UPDATE DNS **"));
		if (mResolverPool.count()>0)
		{
			OpenNICResolverPool proposed;
			log(tr("Scoring resolver pool."));
			mResolverPool.score();
			log(tr("Sorting resolver pool."));
			mResolverPool.sort();
			log(tr("Proposing (")+QString::number(resolverCount)+tr(") candidates."));
			for(int n=0; n < mResolverPool.count() && n < resolverCount; n++)
			{
				OpenNICResolver* resolver = mResolverPool.at(n);
				proposed.append(resolver);
			}
			/** see if what we are proposing is much different than what we have cach'ed already... */
			if ( shouldReplaceWithProposed(proposed) )
			{
				log(tr("Proposal accepted."));
				if ( !replaceActiveResolvers(proposed) )
				{
					log(tr("** Warning: A problem occured while activating resolver cache"));
				}
				rc=mResolverCache.count();
			}
			else
			{
				log(tr("Proposal declined"));
			}
		}
		else
		{
			log(tr("Resolver pool is empty"));
		}
		mUpdatingDNS=false;
	}
	return rc;
}

/**
  * @brief test the integrity of the resolver cache. look for dead resolvers.
  */
bool OpenNICServer::testResolverCache()
{
	for(int n=0; n < mResolverCache.count(); n++ )
	{
		OpenNICResolver* resolver = mResolverCache.at(n);
		if ( !resolver->alive() )
		{
			log("** ACTIVE RESOLVER "+resolver->hostAddress().toString()+"' NOT RESPONDING **");
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
	if ( !mResolversInitialized )							/* have we started any resolvers yet? */
	{
		coldBoot();											/* start from scratch */
	}
	if (force || mResolverCache.count()==0 || mResolverCache.count() < resolverCacheSize() )
	{
		updateDNS(resolverCacheSize());
	}
	else if (mResolverCache.count() && !testResolverCache())		/* how are our currently active resolvers doing?.. */
	{
		updateDNS(resolverCacheSize());						/* ...not so good, get new ones. */
	}
}

/**
  * run regular functions
  */
void OpenNICServer::runOnce()
{
	readSettings();
	refreshResolvers();										/* try to be smart */
	if (mSessions.count())
	{
		purgeDeadSesssions();								/* free up closed gui sessions */
		pruneLog();											/* don't let the log get out of hand */
	}
}

/**
 * @brief Delay for some seconds
 */
void OpenNICServer::delay(quint64 seconds)
{
	QEventLoop loop;
	mSeconds=0L;
	while( seconds > mSeconds )
	{
		loop.processEvents();
	}
}

/**
  * @brief get here on timed events.
  */
void OpenNICServer::timerEvent(QTimerEvent* e)
{
	/* are we anabled? */
	bool enabled = mSystem?mSystem->enabled():true;			

	if ( GlobalShutdown )
	{
		mSystem->shutdown();
		log("** SHUTDOWN **");
		exit(0);
	}

	if ( e->timerId() == mFastTimer )
	{
		if ( enabled )
		{
			runOnce();								/* don't let the log get out of hand */
		}
		else
		{
			readSettings();							/* even disabled, we want to keep settings current */
		}
	}
	else if ( e->timerId() == mBootstrapTimer )		/* higher frequency bootstrap timer to assist in stabalizing */
	{
		if ( enabled )
		{
			refreshResolvers(true);
		}
		if(mBootstrapTicks ++ > BOOTSTRAP_TICKS)	/* kill off this timer after a little bit... */
		{
			killTimer(mBootstrapTimer);
			mBootstrapTimer=(-1);
		}
	}
	else if ( e->timerId() == mRefreshTimer )		/* get here once in a while, a slow timer... */
	{
		if ( enabled )
		{
			refreshResolvers(true);					/* force a resolver cache refresh */
		}
	}
	else if ( e->timerId() == mSecondTimer )
	{
		++mSeconds;									/* keep track of elapsed time */
	}
	else
	{
		inherited::timerEvent(e);
	}
}




