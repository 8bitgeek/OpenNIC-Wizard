/*
 * Copyright (c) 2012 Mike Sharkey <mike@pikeaero.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicresolver.h"

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

OpenNICResolver::OpenNICResolver(QObject *parent, threads)
: QObject(parent)
{
	setThreads(threads);
	mMinuteTimer = startTimer(1000*60);
}

OpenNICResolver::~OpenNICResolver()
{
	closeThreads();
}

/**
  * @brief End threads and cleanup.
  */
void closeThreads()
{
	int count = mThreads.count();
	for(int n=0; n < count; n++)
	{
		OpenNICTest* thread = mThreads.take(0);
		thread->quit();
		delete thread;
	}
	mTherads.clear();

}

/**
  * @brief start threads.
  */
void OpenNICResolver::setThreads(int count)
{
	if ( threads() != count )
	{
		closeThreads();
	}
}

/**
  * @brief Test results come in here.
  */
void OpenNICResolver::insertResult(OpenNICTest::query *result)
{
	if ( result != NULL )
	{
		QString ip = result->addr;
		quint64 latency = result->latency;
		QMutableHashIterator<quint64,QString>i(mResolvers);
		while (i.hasNext())
		{
			i.next();
			if ( i.value() == ip )
			{
				i.remove();
				mResolvers.insert(latency,ip);
			}
			ips.append(i.value());
		}
	}
}

/**
  * @brief Evaluate Candidates
  * @param The number of resolvers to evaluate at a time.
  */
void OpenNICResolver::evaluateResolvers(int count)
{

}


/**
  * @brief Get a default T1 list.
  */
QStringList OpenNICResolver::defaultT1List()
{
	QStringList t1defaults;
	t1defaults << "72.232.162.195";
	t1defaults << "216.87.84.210";
	t1defaults << "199.30.58.57";
	t1defaults << "128.177.28.254";
	t1defaults << "207.192.71.13";
	t1defaults << "66.244.95.11";
	t1defaults << "178.63.116.152";
	t1defaults << "202.83.95.229";
	return t1defaults;
}

/**
  * @brief Fetch the list of DNS resolvers and return them as strings.
  */
QStringList OpenNICResolver::getResolverList()
{
	QStringList ips;
	/* If there are current no resolvers in the hash table, then try to populate it... */
	if ( mResolvers.isEmpty() )
	{
		initializeResolvers();		/* fetch the intial list of T2s */
	}
	if ( mResolvers.isEmpty() )
	{
		ips = defaultT1List();		/* something is wrong - return the T1s */
	}
	else
	{
		/** sort the latenct times in ascending order */
		qSort(mResolvers.begin(),mResolvers.end());
		QMutableHashIterator<quint64,QString>i(mResolvers);
		while (i.hasNext())
		{
			i.next();
			ips.append(i.value());
		}
	}
	return ips;
}

/**
  * @brief Add a dns entry to the system's list of DNS resolvers.
  */
QString OpenNICResolver::addResolver(QString dns,int index)
{
	QString rc;
	QEventLoop loop;
	QString program = "netsh";
	QStringList arguments;
	if ( index == 1 )
	{
		arguments << "interface" << "ip" << "set" << "dns" << "Local Area Connection" << "static" << dns;
	}
	else
	{
		arguments << "interface" << "ip" << "add" << "dns" << "Local Area Connection" << dns << "index="+QString::number(index);
	}
	QProcess *process = new QProcess(this);
	process->start(program, arguments);
	while (process->waitForFinished(3000))
	{
		loop.processEvents();
	}
	rc = process->readAllStandardOutput().trimmed() + "\n";
	delete process;
	return rc;
}

/**
  * @brief Get the text which will show the current DNS resolver settings.
  */
QString OpenNICResolver::getSettingsText()
{
	QByteArray output;
	QEventLoop loop;
	QString program = "netsh";
	QStringList arguments;
	arguments << "interface" << "ip" << "show" << "config" << "Local Area Connection";
	QProcess *process = new QProcess(this);
	process->start(program, arguments);
	while (process->waitForFinished(10000))
	{
		loop.processEvents();
	}
	output = process->readAllStandardOutput();
	delete process;
	return output;
}

/**
  * @brief Fetch the raw list of DNS resolvers and return them as strings.
  */
QStringList OpenNICResolver::getBootstrapResolverList()
{
	QStringList outputList;
	QStringList ips;
	QEventLoop loop;
	QString program = "dig";
	QStringList arguments;
	QString output;
	arguments << "dns.opennic.glue" << "+short";
	QProcess *process = new QProcess(this);
	process->start(program, arguments);
	while (process->waitForFinished(10000))
	{
		loop.processEvents();
	}
	output = process->readAllStandardOutput();
	if (output.isEmpty())
	{
		output = process->readAllStandardError();
	}
	outputList = output.trimmed().split('\n');
	for(int n=0; n < outputList.count(); n++)
	{
		QString ip = outputList.at(n).trimmed();
		if (ip.at(0) >= '0' && ip.at(0) <= '9')
		{
			ips.append(ip);
		}
	}
	return ips;
}

/**
  * @brief Fetch the raw list of resolvers and insert into the hash table.
  */
void OpenNICResolver::initializeResolvers()
{
	QStringList ips = getBootstrapResolverList();
	for(quint64 n=0; n < ips.count(); n++)
	{
		QString ip = ips.at(n);
		mResolvers.insert(n,ip);	/* simulate latency for the initial bootstrap */
	}
}

/**
  * @brief Get here on timer events
  */
void OpenNICResolver::timerEvent(QTimerEvent* e)
{
	if ( e->timerId() == mMinuteTimer )
	{
		evaluateCandidates();
	}
}

