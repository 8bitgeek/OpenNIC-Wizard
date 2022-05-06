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
#ifndef OPENNICSYSTEM_H
#define OPENNICSYSTEM_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QMultiMap>
#include <QTimerEvent>
#include <QHostAddress>
#include <QNetworkConfiguration>
#include <QNetworkInterface>
#include <QList>

#include "opennicdomainname.h"
#include "opennicdomainnamepool.h"

#define OPENNIC_T1_BOOTSTRAP		"bootstrap.t1"
#define	OPENNIC_DOMAINS_BOOTSTRAP	"bootstrap.domains"

class OpenNICSystem
{
	public:

        OpenNICSystem(bool enabled,QString networkInterface);
        virtual ~OpenNICSystem();

        static OpenNICSystem*   instance() {return mInstance;}

        int						random(int low, int high) {return (qrand()%((high+1)-low)+low);}
        bool					backup(QString filename);
        bool					writeStringListToFile(QString filename,QStringList list);
        bool					fileCopy(QString from, QString to);
        bool					fileExists(QString file);
        bool					saveBootstrapT1List(QStringList tiList);
        QStringList				getBootstrapT1List();
        QStringList				getBootstrapT2List();
        bool					saveTestDomains(QStringList domains);
        OpenNICDomainNamePool   getTestDomains();
        OpenNICDomainName		randomDomain();
        QStringList             interfaceNames();
        QString                 defaultInterfaceName();
        QString                 interfaceName();
        void                    setInterfaceName(QString interfaceName);
        bool                    enabled();
        void                    setEnabled(bool enabled);
        virtual void            startup()=0;
        virtual void            shutdown()=0;
        virtual QStringList     getSystemResolverList()=0;
        virtual bool            beginUpdateResolvers(QString& output)=0;
        virtual int             updateResolver(QHostAddress& dns,int index, QString& output)=0;
        virtual bool            endUpdateResolvers(QString& output)=0;

        virtual QString         bootstrapT1Path()=0;
        virtual QString         bootstrapDomainsPath()=0;

        
    protected:
        QList<QNetworkInterface>    interfaces();
        QStringList             parseIPV4Strings(QString input);

    private:
        bool                    mEnabled;
        QString                 mInterfaceName;
        static OpenNICSystem*   mInstance;
        OpenNICDomainNamePool   mTestDomains;
};

#endif // OPENNICSYSTEM_H
