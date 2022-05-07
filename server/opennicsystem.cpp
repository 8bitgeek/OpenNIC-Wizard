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

OpenNICSystem* OpenNICSystem::mInstance=NULL;

OpenNICSystem::OpenNICSystem(bool enabled,QString networkInterface)
: mEnabled(enabled)
{
    while(mInstance); /* singleton trap */
    mInstance = this;
	if ( networkInterface.length() ==  0)
	{
		mInterfaceName = defaultInterfaceName();
	}
}

OpenNICSystem::~OpenNICSystem()
{
    mInstance=NULL;
}

bool OpenNICSystem::enabled()
{
	return mEnabled;
}

void OpenNICSystem::setEnabled(bool enabled)
{
	if (mEnabled!=enabled)
	{
		/* 
		* Only emit startuo/shutdown upon state change.
		*/
		mEnabled=enabled;
		if( mEnabled )
		{
			OpenNICServer::log("** ENABLEDD -> STARTUP **");
			startup();
		}
		else
		{
			OpenNICServer::log("** DISABLED -> SHUTDOWN **");
			shutdown();
		}
	}
}

/**
 * @brief OpenNICSystem_Linux::interfaces
 * @return List of interfaces.
 */
QList<QNetworkInterface> OpenNICSystem::interfaces()
{
    return QNetworkInterface::allInterfaces();
}

QStringList OpenNICSystem::interfaceNames()
{
	QStringList rc;
    QList<QNetworkInterface> interfaceList = interfaces();
    for(int n=0; n < interfaceList.count(); n++)
    {
        QNetworkInterface iface = interfaceList[n];
        rc << interfaceList[n].humanReadableName();
    }
	return rc;
}

QString OpenNICSystem::defaultInterfaceName()
{
	QStringList names = interfaceNames();
	if ( names.count() )
		return names[0];
	return "";
}

QString OpenNICSystem::interfaceName()
{
	if ( mInterfaceName.isEmpty() )
		mInterfaceName = defaultInterfaceName();
	return mInterfaceName;
}

void OpenNICSystem::setInterfaceName(QString interfaceName)
{
	mInterfaceName=interfaceName;
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
  * @return true if file exists
  */
bool OpenNICSystem::fileExists(QString file)
{
	return QFile::exists(file);
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
	return writeStringListToFile(bootstrapT1Path(),list);
}

/**
  * @brief re-write the domains file
  */
bool OpenNICSystem::saveTestDomains(QStringList list)
{
	return writeStringListToFile(bootstrapDomainsPath(),list);
}

/**
  * @brief Get a default T1 list from the bootstrap file.
  * @return A string list of IP numbers representing potential T1s.
  */
QStringList OpenNICSystem::getBootstrapT1List()
{
	QStringList rc;
	QFile file(bootstrapT1Path());
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
		rc << "161.97.219.84";
		rc << "163.172.168.171";
		rc << "94.103.153.176";
		rc << "207.192.71.13";
		rc << "178.63.116.152";
		rc << "209.141.36.19";
		rc << "188.226.146.136";
		rc << "198.98.51.33";
		rc << "79.124.7.81";
		rc << "144.76.103.143";
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
		QFile file(bootstrapDomainsPath());
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
 * @brief Given some input text, parse out the IPV4 adddresses.
 * 
 * @param input String containing noise and IPV4 address text.
 * @return QStringList of IPV4 addresses
 */
QStringList OpenNICSystem::parseIPV4Strings(QString input)
{
	QStringList result;
	QString temp;
	QStringList lines = input.split('\n');
	for( int y=0; y < lines.count(); y++)
	{
		QString line = lines[y].simplified().trimmed();
		if ( !line.isEmpty() && line[0] != '#' && line[0] != ';' && line.indexOf('.')>0 )
		{
			for( int x=0; x < line.length(); x++ )
			{
				QChar ch = line[x];
				if ( ch == '\n' || ch == '\t' || ch == ' ' || ch == '.' || (ch >= '0' && ch <= '9') )
				{
					temp += ch;
				}
			}
			result << temp;
			temp.clear();
		}
	}
	return result;
}
