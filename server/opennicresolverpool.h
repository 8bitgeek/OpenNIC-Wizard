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
#ifndef OPENNICRESOLVERPOOL_H
#define OPENNICRESOLVERPOOL_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QTimerEvent>
#include <QScriptEngine>
#include <QScriptValue>
#include "opennicresolver.h"

class OpenNICResolverPool : public QObject
{
		Q_OBJECT
	public:
		OpenNICResolverPool(QObject *parent = 0);
		OpenNICResolverPool(const OpenNICResolverPool& other);
		virtual ~OpenNICResolverPool();

		OpenNICResolverPool&			copy(OpenNICResolverPool& other);
		bool							contains(OpenNICResolver* item);
		bool							contains(QHostAddress item);
		QList<OpenNICResolver*>&		resolvers()		{return mResolvers;}
		int								count()			{return mResolvers.count();}
		OpenNICResolver*				at(int pos)		{return mResolvers[pos];}
		bool							append(OpenNICResolver* item);
		bool							append(QHostAddress& host,QString kind);
		bool							insort(OpenNICResolver* item);
		bool							insort(QHostAddress& host,QString kind="");
		int								indexOf(OpenNICResolver* item);
		int								indexOf(QHostAddress hostAddress);
		QStringList						toStringList();
		QStringList						toStringList(QString select);
		OpenNICResolverPool&			fromStringList(const QStringList strings, QString kind="");
		OpenNICResolverPool&			fromIPList(const QStringList ips,QString kind);
		OpenNICResolverPool&			operator<<(const QStringList& strings);
	public slots:
		void							score();
		void							setMaxHistoryDepth(int maxHistoryDepth);
		void							randomize();
		void							sort();
		void							clear();
	private slots:
		void							signalHandlerException(const QScriptValue& exception);
	private:
		double							latency(double& min, double& max);
		double							scoreResolverInternal(OpenNICResolver* resolver, double averagePoolLatency, double minPoolLatency, double maxPoolLatency );
		double							scoreResolverScript(OpenNICResolver* resolver, double averagePoolLatency, double minPoolLatency, double maxPoolLatency );
		void							swap(int a,int b);
		QList<OpenNICResolver*>			mResolvers;
		QScriptEngine					mScriptEngine;
};


#endif // OPENNICRESOLVERPOOL_H
