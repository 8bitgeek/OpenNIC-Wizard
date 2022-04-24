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

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QSettings>
#include <QSpinBox>
#include <QFile>
#include <QIODevice>
#include <QDateTime>
#include <QNetworkConfigurationManager>

#define RESOLVE_CONF            "/etc/resolv.conf"
#define RESOLVE_CONF_BACKUP     "/var/resolv.conf.bak"

#ifdef SIMULATE
QString sResolvConf;
#endif

#define inherited OpenNICSystem

OpenNICSystem_Linux::OpenNICSystem_Linux()
: inherited::OpenNICSystem()
{
}

/**
 * @brief OpenNICSystem_Linux::beginUpdateResolvers
 * @param output
 * @return
 */
bool OpenNICSystem_Linux::beginUpdateResolvers(QString& output)
{
    output.clear();
    mInterfaces = interfaces();
	return true;
}

/**
  * @brief Add a dns entry to the system's list of DNS resolvers.
  * @param resolver The IP address of teh resolver to add to the system
  * @param index resolver sequence (1..n)
  */
int OpenNICSystem_Linux::updateResolver(QHostAddress& resolver,int index,QString& output)
{
    #ifdef SIMULATE
            if (index == 1) sResolvConf.clear();
            sResolvConf += "nameserver "+resolver.toString()+"\n";
            output=resolver.toString();
    #else
        QFile file(RESOLVE_CONF);
        if ( (index==1) ? file.open(QIODevice::ReadWrite|QIODevice::Truncate) : file.open(QIODevice::ReadWrite|QIODevice::Append) )
        {
            QString line("nameserver "+resolver.toString()+"\n");
            file.write(line.toLocal8Bit());
            file.close();
            output=resolver.toString();
        }
        return 0;
    #endif
}

bool OpenNICSystem_Linux::endUpdateResolvers(QString& /* output */)
{
	return true;
}

/**
  * @brief Get the text which will show the current DNS resolver settings.
  */
QString OpenNICSystem_Linux::getSystemResolverList()
{
    #ifdef SIMULATE
        return sResolvConf;
    #else
        QFile file(RESOLVE_CONF);
        if ( file.open(QIODevice::ReadOnly) )
        {
            QString text(file.readAll());
            file.close();
            return text;
        }
        return "Could not obtain system resolver list.";
    #endif
}

/**
 * @brief Preserve the resolver cache /etc/resolv.conf to /etc/resolv.conf.bak
 * @return true 
 * @return false 
 */
bool OpenNICSystem_Linux::preserveResolverCache()
{    
    bool rc=false;
    QFile resolv_conf(RESOLVE_CONF);
    QFile resolv_conf_bak(RESOLVE_CONF_BACKUP);

    if ( resolv_conf_bak.open( QIODevice::ReadWrite|QIODevice::Truncate ) )
    {
        if ( resolv_conf.open( QIODevice::ReadOnly ) )
        {
            QByteArray cache = resolv_conf.readAll();
            rc = (resolv_conf_bak.write(cache) == cache.size() ) ? true : false;
            resolv_conf.close();
        }
        resolv_conf_bak.close();
    }
    return rc;
}

/**
 * @brief Restore the resolver cache /etc/resolv.conf.bak to /etc/resolv.conf
 * @return true 
 * @return false 
 */
bool OpenNICSystem_Linux::restoreResolverCache()
{
    bool rc=false;
    QFile resolv_conf(RESOLVE_CONF);
    QFile resolv_conf_bak(RESOLVE_CONF_BACKUP);

    if ( resolv_conf_bak.open( QIODevice::ReadOnly ) )
    {
        if ( resolv_conf.open( QIODevice::ReadWrite|QIODevice::Truncate ) )
        {
            QByteArray cache = resolv_conf_bak.readAll();
            rc = (resolv_conf.write(cache) == cache.size() ) ? true : false;
            resolv_conf.close();
        }
        resolv_conf_bak.close();
    }
    return rc;
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





