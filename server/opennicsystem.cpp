/*
 * This file is a part of OpenNIC Wizard
 * Copywrong (c) 2012-2018 Mike Sharkey
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
#include <QNetworkConfigurationManager>

OpenNICSystem* OpenNICSystem::mInstance=NULL;

OpenNICSystem::OpenNICSystem()
{
    while(mInstance); /* FIXME - singleton trap */
    mInstance = this;
}

OpenNICSystem::~OpenNICSystem()
{
    mInstance=NULL;
}

/**
  * @brief file copy
  */
bool OpenNICSystem::fileCopy(QString from, QString to)
{
	if ( from != to && !from.isEmpty() && !to.isEmpty() )
	{
		QFile fFrom(from);
		QFile fTo(to);
		if (fFrom.open(QIODevice::ReadOnly))
		{
			if(fTo.open(QIODevice::ReadWrite))
			{
				fTo.write(fFrom.readAll());
				fTo.close();
			}
			fFrom.close();
			return true;
		}
	}
	return false;
}

/**
  * @brief backup a file
  */
bool OpenNICSystem::backup(QString filename)
{
	return fileCopy(filename,QDateTime::currentDateTime().toString("yyMMddhhmmss")+filename+".bak");
}

/**
  * @brief write a string list to a file
  */
bool OpenNICSystem::writeStringListToFile(QString filename,QStringList list)
{
	if (backup(filename))
	{
		QFile file(filename);
		if ( file.open(QIODevice::ReadWrite|QIODevice::Truncate) )
		{
			for(int n=0; n < list.count(); n++)
			{
                QByteArray line = list.at(n).toLocal8Bit();
				line += '\n';
				file.write(line);
			}
			file.close();
			return true;
		}
	}
	return false;
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

/**
  * @brief Get a default T1 list from the bootstrap file.
  * @return A string list of IP numbers representing potential T1s.
  */
QStringList OpenNICSystem::getBootstrapT1List()
{
	QStringList rc;
	QFile file(OPENNIC_T1_BOOTSTRAP);
	if ( file.open(QIODevice::ReadOnly) )
	{
		while (!file.atEnd()) {
			QByteArray line = file.readLine();
			QString ip(line);
			if ( !ip.trimmed().isEmpty() )
			{
				rc << ip.trimmed();
			}
		}
		file.close();
	}
	if ( !rc.count() )
	{
		/** a last ditch effort... */
		rc << "72.232.162.195";
		rc << "216.87.84.210";
		rc << "199.30.58.57";
		rc << "128.177.28.254";
		rc << "207.192.71.13";
		rc << "66.244.95.11";
		rc << "178.63.116.152";
		rc << "202.83.95.229";
	}
	return rc;
}

/**
  * @brief Fetch the raw list of DNS resolvers and return them as strings.
  */
QStringList OpenNICSystem::getBootstrapT2List()
{
	QStringList outputList;
	QStringList ips;
	QEventLoop loop;
	QString program = "dig";
	QStringList arguments;
	QString output;
	arguments << "dns.opennic.glue" << "+short";
	QProcess* process = new QProcess();
	process->start(program, arguments);
	while (process->waitForFinished(15000))
	{
		loop.processEvents();
	}
	output = process->readAllStandardOutput();
	outputList = output.trimmed().split('\n');
	for(int n=0; n < outputList.count(); n++)
	{
		QString ip = outputList.at(n).trimmed();
		if ( ip.length() )
		{
			if (ip.at(0) >= '0' && ip.at(0) <= '9')
			{
				ips.append(ip);
			}
		}
	}
	delete process;
	return ips;
}

/**
  * @brief Get a domains list from the bootstrap file.
  * @return A string list of domains to test.
  */
OpenNICDomainNamePool OpenNICSystem::getTestDomains()
{
	if ( mTestDomains.count() == 0 )
	{
		QStringList rc;
		QFile file(OPENNIC_DOMAINS_BOOTSTRAP);
		if ( file.open(QIODevice::ReadOnly) )
		{
			while (!file.atEnd())
			{
				QString line = file.readLine();
				if (!line.trimmed().isEmpty())
				{
					OpenNICDomainName domain(line.trimmed());
					mTestDomains.append(domain);
				}
			}
			file.close();
		}
		else
		{
			/** a last ditch effort... */
			rc << "icann;wikipedia.org";
			rc << "icann;www.abs.gov.au";
			rc << "icann;yahoo.com";
			rc << "icann;google.com";
			rc << "icann;360.cn";
			rc << "icann;canada.ca";
			rc << "opennic;dns.opennic.glue";
			rc << "opennic;grep.geek";
			rc << "opennic;opennic.glue";
			rc << "opennic;reg.for.free";
			rc << "opennic;register.bbs";
			rc << "opennic;register.fur";
			rc << "opennic;register.gopher";
			rc << "opennic;register.ing";
			for(int n=0; n < rc.count(); n++)
			{
				OpenNICDomainName domain(rc[n]);
				mTestDomains.append(domain);
			}
		}
	}
	return mTestDomains;
}

/**
  * @brief retrieve a random domain
  */
OpenNICDomainName OpenNICSystem::randomDomain()
{
	OpenNICDomainNamePool domains = getTestDomains();
	int n = random(0,domains.count()-1);
	return domains.at(n);
}

/**
 * @brief OpenNICSystem_Linux::interfaces
 * @return List of interfaces.
 */
QList<QNetworkConfiguration> OpenNICSystem::interfaces()
{
    QList<QNetworkConfiguration> rc;
    QNetworkConfigurationManager manager;
    rc = manager.allConfigurations();
    return rc;
}

