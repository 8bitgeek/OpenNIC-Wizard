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
#ifndef OPENNICRESOLVERTEST_H
#define OPENNICRESOLVERTEST_H

#include <QThread>
#include <QList>
#include <QMutex>
#include <QDateTime>
#include <QHostAddress>
#include <QTimerEvent>

#include "opennicdnsquery.h"
#include "opennicdnsclient.h"
#include "opennicdomainname.h"

#define PURGE_TIMEOUT	30		/* purge timeout in seconds */

class OpenNICResolverTest : public OpenNICDnsClient
{
	Q_OBJECT
	public:
		OpenNICResolverTest(bool active=true, QObject *parent=0);
		virtual ~OpenNICResolverTest();
	protected slots:
		virtual void					reply(OpenNICDnsQuery& query);
		virtual	void					setInterval(int seconds);
	protected:
		virtual void					purge() {}
		virtual	void					test() {}
		virtual void					resolve(QHostAddress addr, OpenNICDomainName name, quint16 port=DEFAULT_DNS_PORT);
		virtual void					timerEvent(QTimerEvent* e);
	private:
		int								mSecondTimer;
		int								mTimerInterval;
		int								mTimerCount;
};


#endif // OPENNICRESOLVERTEST_H
