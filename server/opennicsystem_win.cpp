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
#include "opennicsystem.h"

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


OpenNICDomainNamePool OpenNICSystem::mTestDomains;

#define inherited OpenNICSystem

OpenNICSystem_Win::OpenNICSystem_Win()
: inherited::OpenNICSystem()
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
    mInterfaces = interfaces();
	return true;
}

int OpenNICSystem_Win::updateResolver(QHostAddress& dns,int index, QString& output)
{

}

bool OpenNICSystem_Win::endUpdateResolvers(QString& output)
{

}

/**
  * @brief re-wriet the T1 bootstrap file
  */
bool OpenNICSystem::saveBootstrapT1List(QStringList list)
{
	return writeStringListToFile(OPENNIC_T1_BOOTSTRAP,list);
}

/**
  * @brief re-write the domains file
  */
bool OpenNICSystem::saveTestDomains(QStringList list)
{
	return writeStringListToFile(OPENNIC_DOMAINS_BOOTSTRAP,list);
}

QString OpenNICSystem::bootstrapT1Path()
{
    return OPENNIC_T1_BOOTSTRAP;
}

QString OpenNICSystem::bootstrapDomainsPath()
{
    return OPENNIC_DOMAINS_BOOTSTRAP;

}

bool OpenNICSystem::beginUpdateResolvers(QString& output)
{
	/* on windows nothing to do here */
	return true;
}

/**
  * @brief Add a dns entry to the system's list of DNS resolvers.
  * @param resolver The IP address of teh resolver to add to the system
  * @param index resolver sequence (1..n)
  */
int OpenNICSystem::updateResolver(QHostAddress& resolver,int index,QString& output)
{
	int rc;
	QEventLoop loop;
	QString program = "netsh";
	QStringList arguments;
	if ( ++index == 1 ) /* on windows(tm) index starts at 1 */
	{
		arguments << "interface" << "ip" << "set" << "dns" << "Local Area Connection" << "static" << resolver.toString();
	}
	else
	{
		arguments << "interface" << "ip" << "add" << "dns" << "Local Area Connection" << resolver.toString() << "index="+QString::number(index);
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

bool OpenNICSystem::endUpdateResolvers(QString& output)
{
	return true;
}

/**
  * @brief Get the text which will show the current DNS resolver settings.
  */
QString OpenNICSystem::getSystemResolverList()
{
	QByteArray output;
	QEventLoop loop;
	QString program = "netsh";
	QStringList arguments;
	arguments << "interface" << "ip" << "show" << "config" << "Local Area Connection";
	QProcess* process = new QProcess();
	process->start(program, arguments);
	while (process->waitForFinished(10000))
	{
		loop.processEvents();
	}
	output = process->readAllStandardOutput();
	delete process;
	if (output.trimmed().isEmpty())
	{
		return "Could not obtain system resolver list.";
	}
	return output;
}

#endif




