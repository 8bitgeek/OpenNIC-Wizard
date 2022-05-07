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
#include "opennicsystem_win.h"
#include "opennicserver.h"

#include <QObject>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QEventLoop>
#include <QProgressDialog>
#include <QByteArray>
#include <QSettings>
#include <QSpinBox>
#include <QFile>
#include <QIODevice>
#include <QDateTime>
#include <QNetworkInterface>
#include <QTextStream>

#define inherited OpenNICSystem

#define RESOLVE_CONF            "resolv.conf"
#define RESOLVE_CONF_BACKUP     "resolv.conf.bak"

OpenNICSystem_Win::OpenNICSystem_Win(bool enabled,QString networkInterface)
: inherited::OpenNICSystem(enabled,networkInterface)
{
}

void OpenNICSystem_Win::startup()
{
    if ( preserveResolverCache() )
        OpenNICServer::log("resolver cache preserved");
    else
        OpenNICServer::log("failed to preserved resolver cache");
}

void OpenNICSystem_Win::shutdown()
{
    if ( restoreResolverCache() )
        OpenNICServer::log("resolver cache restored");
    else
        OpenNICServer::log("failed to restore resolver cache");
}

bool OpenNICSystem_Win::beginUpdateResolvers(QString& output)
{
    output.clear();
	return true;
}

/**
  * @brief Add a dns entry to the system's list of DNS resolvers.
  * @param resolver The IP address of teh resolver to add to the system
  * @param index resolver sequence (1..n)
  */
int OpenNICSystem_Win::updateResolver(QHostAddress& resolver,int index,QString& output)
{
	int rc;
	QEventLoop loop;
	QString program = "netsh";
	QStringList arguments;
	/* 
	 * MS Windows(tm) index starts at 1 
	 */
	if ( ++index == 1 ) 
	{
		arguments << "interface" << "ip" << "set" << "dns" << interfaceName() << "static" << resolver.toString();
	}
	else
	{
		arguments << "interface" << "ip" << "add" << "dns" << interfaceName() << resolver.toString() << "index="+QString::number(index);
	}
	QProcess* process = new QProcess();
	process->start(program, arguments);
	while (process->waitForFinished(10000))
	{
		loop.processEvents();
	}
	output = process->readAllStandardOutput().trimmed() + "\n";
	rc = process->exitCode();
	delete process;
	return rc;
}

bool OpenNICSystem_Win::endUpdateResolvers(QString& output)
{
	return true;
}

QString OpenNICSystem_Win::bootstrapT1Path()
{
    return OPENNIC_T1_BOOTSTRAP;
}

QString OpenNICSystem_Win::bootstrapDomainsPath()
{
    return OPENNIC_DOMAINS_BOOTSTRAP;

}


/**
  * @brief Get the text which will show the current DNS resolver settings.
  */
QStringList OpenNICSystem_Win::getSystemResolverList()
{
	int x;
	QString temp;
	QByteArray stdoutText;
	QEventLoop loop;
	QString program = "netsh";
	QStringList arguments;
	arguments << "interface" << "ip" << "show" << "config" << interfaceName();
	QProcess* process = new QProcess();
	process->start(program, arguments);
	while (process->waitForFinished(10000))
	{
		loop.processEvents();
	}
	stdoutText = process->readAllStandardOutput();
	delete process;
	if (stdoutText.trimmed().isEmpty())
	{
		QStringList result;
		result << QString("Could not obtain system resolver list.");
		return result;
	}
	/** 
	 * strip out noise and return ipv4 numbers list. 
	 */
	temp = QString(stdoutText);
	x = temp.indexOf("DNS Servers:");
	return parseIPV4Strings(temp.right(temp.length()-(x+QString("DNS Servers:").length())));
}

/**
 * @brief Preserve the resolver cache ./resolv.conf to ./resolv.conf.bak
 *        and repopulate ./resolv.conf
 * @return true 
 * @return false 
 */
bool OpenNICSystem_Win::preserveResolverCache()
{   
	QStringList resolvers = getSystemResolverList();
	if ( resolvers.count() )
	{
		fileCopy(RESOLVE_CONF,RESOLVE_CONF_BACKUP);
		QFile file(RESOLVE_CONF);
		if(file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QString buffer = resolvers.join('\n');
			QTextStream out(&file);
    		out << buffer;
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
 * @brief Restore the resolver cache ./resolv.conf.bak to ./resolv.conf
 * @return true 
 * @return false 
 */
bool OpenNICSystem_Win::restoreResolverCache()
{
	QFile file(RESOLVE_CONF);
	if (file.open(QIODevice::ReadOnly))
	{
		int index=0;
		while(!file.atEnd())
		{
			QString buffer = file.readLine().simplified().trimmed();
			if ( !buffer.isEmpty() )
			{
				QString output;
				QHostAddress address(buffer);
				updateResolver(address,index++,output);
			}
		}
		file.close();
	}
	else
	{
		return false;
	}
    return true; 
}

QString OpenNICSystem_Win::defaultInterfaceName()
{
    QString interfaceName = inherited::defaultInterfaceName();
    if ( interfaceName.isEmpty() )
        interfaceName = "Local Area Connection";
    return interfaceName;
}
