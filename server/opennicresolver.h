/*
 *   This file is a part of OpenNIC Wizard
 *   Copyright (C) 2012-2015  Mike Sharkey <mike@8bitgeek.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef OPENNICRESOLVER_H
#define OPENNICRESOLVER_H

#include <QObject>
#include <QHostAddress>
#include <QDateTime>
#include <QString>
#include <QTimerEvent>

#include "opennicdnsquery.h"
#include "opennicdnsquerylistener.h"

#define RESOLVER_SAMPLE_STORAGE_LIMIT		10			/* number of samples to remember for averaging */

class OpenNICResolver : public OpenNICDnsQueryListener
{
	Q_OBJECT
	public:
		OpenNICResolver(QObject *parent = 0);
		OpenNICResolver(bool active, QObject *parent = 0);
		OpenNICResolver(QHostAddress hostAddress, QString kind="", QObject* parent=NULL);
		OpenNICResolver(const OpenNICResolver& other);
		virtual ~OpenNICResolver();
		int							incRef();
		int							decRef();
		int							refCount();
		OpenNICResolver&			copy(const OpenNICResolver& other);
		OpenNICResolver&			operator=(const OpenNICResolver& other);
		bool						operator==(OpenNICResolver &other);
		bool						operator!=(OpenNICResolver &other);
		bool						operator>(OpenNICResolver &other);
		bool						operator<(OpenNICResolver &other);
		bool						operator>=(OpenNICResolver &other);
		bool						operator<=(OpenNICResolver &other);
		QHostAddress&				hostAddress();
		int							testCount();
		int							replyCount();
		int							timeoutCount();
		int							lastLatency();
		double						averageLatency();
		QDateTime					lastReply();
		QDateTime					lastTimeout();
		QString						lastFault();
		bool						alive();
		double						score();
		void						setScore(double score);
		QString&					kind();
		void						setKind(QString kind);
		OpenNICDnsQuery*			mostRecentQuery();
		bool						resolvesNIC(QString nic);
		QString&					toString();
	protected slots:
		virtual void				starting(OpenNICDnsQuery* query);
		virtual void				finished(OpenNICDnsQuery* query);
		virtual void				expired(OpenNICDnsQuery* query);
	protected:
		virtual	void				test();
		virtual void				timerEvent(QTimerEvent *);
	public slots:
		virtual void				clear();
	private slots:
		void						resetQueryTimer();
	private:
		int							mRefCount;			/* a count of the number of references */
		double						mScore;				/* the score realtive to other resolvers in the pool */
		QHostAddress				mHostAddress;		/* host address wrapper */
		QString						mKind;				/* the kind of resolver */
		int							mQueryIntervalTimer;/* interval timer */
		int							mBootstrapTicks;	/* higher frequency intervals during the bootstrap ticks */
		QString						mString;			/* for returning a string reference toString() */
};


#endif // OpenNICResolver_H
