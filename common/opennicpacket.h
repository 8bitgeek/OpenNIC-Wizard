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
		static const QString	interface_list;					/* network interface list */
		static const QString	interface;						/* network interface */
		static const QString	opennic_enabled;				/* resolvers enabled or not */

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

