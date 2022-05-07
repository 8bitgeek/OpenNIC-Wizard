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
#include "opennicsystem_linux.h"
#include "opennicserver.h"

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QSettings>
#include <QSpinBox>
#include <QFile>
#include <QIODevice>
#include <QDateTime>
#include <QNetworkInterface>

#include <stdlib.h>

#define ENV_OPENNIC_T1_BOOTSTRAP        "OPENNIC_T1_BOOTSTRAP"
#define ENV_OPENNIC_DOMAINS_BOOTSTRAP   "OPENNIC_DOMAINS_BOOTSTRAP"

#define RESOLVE_CONF            "/etc/resolv.conf"
#define RESOLVE_CONF_BACKUP     "/var/resolv.conf.bak"

#ifdef SIMULATE
QString sResolvConf;
#endif

#define inherited OpenNICSystem

OpenNICSystem_Linux::OpenNICSystem_Linux(bool enabled,QString networkInterface)
: inherited::OpenNICSystem(enabled,networkInterface)
{
}

void OpenNICSystem_Linux:: startup()
{
    if ( preserveResolverCache() )
        OpenNICServer::log("resolver cache preserved");
    else
        OpenNICServer::log("failed to preserved resolver cache");
}

void OpenNICSystem_Linux::shutdown()
{
    if ( restoreResolverCache() )
        OpenNICServer::log("resolver cache restored");
    else
        OpenNICServer::log("failed to restore resolver cache");
}

/**
 * @brief OpenNICSystem_Linux::beginUpdateResolvers
 * @param output
 * @return
 */
bool OpenNICSystem_Linux::beginUpdateResolvers()
{
    mResolvers.clear();
	return true;
}

/**
  * @brief Add a dns entry to the system's list of DNS resolvers.
  * @param resolver The IP address of teh resolver to add to the system
  */
bool OpenNICSystem_Linux::updateResolver(QHostAddress& resolver)
{
    mResolvers << resolver.toString();
    return true;
}

bool OpenNICSystem_Linux::endUpdateResolvers()
{
    if ( mResolvers.count() )
    {
        QFile file(RESOLVE_CONF);
        if ( file.open(QIODevice::ReadWrite|QIODevice::Truncate) )
        {
            QString output;
            for(int y=0; y < mResolvers.count(); y++)
            {
                mResolvers[y] = "nameserver " + mResolvers[y];
            }
            output=mResolvers.join('\n') + "\n";
            file.write(output.toLocal8Bit());
            file.close();
        }
        else 
        {
            return false;
        }
    }
	return true;
}

/**
  * @brief Get the text which will show the current DNS resolver settings.
  */
QStringList OpenNICSystem_Linux::getSystemResolverList()
{
    QFile file(RESOLVE_CONF);
    if ( file.open(QIODevice::ReadOnly) )
    {
        QString text(file.readAll());
        file.close();
        return parseIPV4Strings(text);
    }
    QStringList result;
    result << "Could not obtain system resolver list.";
    return result;
}

/**
 * @brief Preserve the resolver cache /etc/resolv.conf to /etc/resolv.conf.bak
 * @return true 
 * @return false 
 */
bool OpenNICSystem_Linux::preserveResolverCache()
{    
    return fileCopy(RESOLVE_CONF,RESOLVE_CONF_BACKUP);
}

/**
 * @brief Restore the resolver cache /etc/resolv.conf.bak to /etc/resolv.conf
 * @return true 
 * @return false 
 */
bool OpenNICSystem_Linux::restoreResolverCache()
{
    return fileCopy(RESOLVE_CONF_BACKUP,RESOLVE_CONF);
}


QString OpenNICSystem_Linux::bootstrapT1Path()
{
    if ( getenv(ENV_OPENNIC_T1_BOOTSTRAP) )
    {
        if ( fileExists( getenv(ENV_OPENNIC_T1_BOOTSTRAP) ) )
            return getenv(ENV_OPENNIC_T1_BOOTSTRAP);
    }

    if ( fileExists( QString("/usr/local/etc/") + OPENNIC_T1_BOOTSTRAP ) )
        return QString("/usr/local/etc/") + OPENNIC_T1_BOOTSTRAP;

    if ( fileExists( QString("/usr/etc/") + OPENNIC_T1_BOOTSTRAP ) )
        return QString("/usr/etc/") + OPENNIC_T1_BOOTSTRAP;

    if ( fileExists( QString("/etc/") + OPENNIC_T1_BOOTSTRAP ) )
        return QString("/etc/") + OPENNIC_T1_BOOTSTRAP;

    if ( fileExists( QString("/opt/opennic/") + OPENNIC_T1_BOOTSTRAP ) )
        return QString("/opt/opennic/") + OPENNIC_T1_BOOTSTRAP;

    return OPENNIC_T1_BOOTSTRAP;
}

QString OpenNICSystem_Linux::bootstrapDomainsPath()
{
    if ( getenv(ENV_OPENNIC_DOMAINS_BOOTSTRAP) )
    {
        if ( fileExists( getenv(ENV_OPENNIC_DOMAINS_BOOTSTRAP) ) )
            return getenv(ENV_OPENNIC_DOMAINS_BOOTSTRAP);
    }

    if ( fileExists( QString("/usr/local/etc/") + OPENNIC_DOMAINS_BOOTSTRAP ) )
        return QString("/usr/local/etc/") + OPENNIC_DOMAINS_BOOTSTRAP;

    if ( fileExists( QString("/usr/etc/") + OPENNIC_DOMAINS_BOOTSTRAP ) )
        return QString("/usr/etc/") + OPENNIC_DOMAINS_BOOTSTRAP;

    if ( fileExists( QString("/etc/") + OPENNIC_DOMAINS_BOOTSTRAP ) )
        return QString("/etc/") + OPENNIC_DOMAINS_BOOTSTRAP;

    if ( fileExists( QString("/opt/opennic/") + OPENNIC_DOMAINS_BOOTSTRAP ) )
        return QString("/opt/opennic/") + OPENNIC_DOMAINS_BOOTSTRAP;

    return OPENNIC_DOMAINS_BOOTSTRAP;
}

QString OpenNICSystem_Linux::defaultInterfaceName()
{
    QString interfaceName = inherited::defaultInterfaceName();
    if ( interfaceName.isEmpty() )
        interfaceName = "eth0";
    return interfaceName;
}
