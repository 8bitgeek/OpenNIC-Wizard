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
#ifndef OPENNICPACKET_H
#define OPENNICPACKET_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QTcpSocket>
#include <QByteArray>

class OpenNICPacket : public QObject
{
    Q_OBJECT
	public:

		static const QString	tcp_listen_port;				/* the tcp port to listen on */
		static const QString	refresh_timer_period;			/* the service refresh timer period */
		static const QString	resolver_cache_size;			/* the resolver cache size */
		static const QString	resolver_pool;					/* the resolver pool */
		static const QString	resolver_cache;					/* the resolver cache */
		static const QString	resolver_history;				/* the resolver query history records */
		static const QString	bootstrap_t1_list;				/* the bootstrap t1 liost */
		static const QString	bootstrap_domains;				/* the bootstrap domains */
		static const QString	system_text;					/* the system text */
		static const QString	journal_text;					/* the journal text */
		static const QString	async_message;					/* an async message */
		static const QString	score_rules;					/* score rules */
		static const QString	score_internal;					/* score internal */
		static const QString	update_dns;						/* force dns update now */
		static const QString	poll_keys;						/* poll for variables */

		OpenNICPacket(QObject *parent = 0);
		virtual ~OpenNICPacket();
		void								set(const QString& name, QVariant value);
		QVariant							get(const QString& name);
		bool								contains(const QString& key);
		QMap<QString,QVariant>&				data() {return mData;}
	signals:
		void								dataReady();
	public slots:
		void								clear();
		void								recv(QTcpSocket* socket);
		void								send(QTcpSocket* socket);
	private:
		QMap<QString,QVariant>				mData;
		QByteArray							mRXData;
		int									mRXState;
		quint32								mRXLength;

};

#endif // OPENNICPACKET_H

