/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICRESOLVERPOOL_H
#define OPENNICRESOLVERPOOL_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QTimerEvent>
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
		QStringList&					toStringList();
		OpenNICResolverPool&			fromStringList(const QStringList strings, QString kind="");
		OpenNICResolverPool&			fromIPList(const QStringList ips,QString kind);
		OpenNICResolverPool&			operator<<(const QStringList& strings);
	public slots:
		void							score();
		void							setMaxHistoryDepth(int maxHistoryDepth);
		void							randomize();
		void							sort();
		void							clear();
	private:
		void							swap(int a,int b);
		QList<OpenNICResolver*>			mResolvers;
		QStringList						mStringList;
};


#endif // OPENNICRESOLVERPOOL_H
