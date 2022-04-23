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

#define OPENNIC_T1_BOOTSTRAP		"bootstrap.t1"
#define	OPENNIC_DOMAINS_BOOTSTRAP	"bootstrap.domains"

OpenNICDomainNamePool OpenNICSystem::mTestDomains;

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

#if defined(Q_OS_WIN32)
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

#elif defined(Q_OS_UNIX)

//#define SIMULATE 1

#ifdef SIMULATE
QString sResolvConf;
#endif

bool OpenNICSystem::beginUpdateResolvers(QString& /* output */)
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
#ifdef SIMULATE
		if (index == 1) sResolvConf.clear();
		sResolvConf += "nameserver "+resolver.toString()+"\n";
		output=resolver.toString();
#else
	QFile file("/etc/resolv.conf");
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

bool OpenNICSystem::endUpdateResolvers(QString& /* output */)
{
	return true;
}

/**
  * @brief Get the text which will show the current DNS resolver settings.
  */
QString OpenNICSystem::getSystemResolverList()
{
#ifdef SIMULATE
	return sResolvConf;
#else
	QFile file("/etc/resolv.conf");
	if ( file.open(QIODevice::ReadOnly) )
	{
		QString text(file.readAll());
		file.close();
		return text;
	}
	return "Could not obtain system resolver list.";
#endif
}

#endif




