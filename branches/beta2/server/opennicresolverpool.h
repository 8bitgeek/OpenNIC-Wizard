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
#include "opennicresolverpoolitem.h"

class OpenNICResolverPool : public QObject
{
		Q_OBJECT
	public:
		OpenNICResolverPool(QObject *parent = 0);
		OpenNICResolverPool(const OpenNICResolverPool& other);
		virtual ~OpenNICResolverPool();

		OpenNICResolverPool&			copy(OpenNICResolverPool& other);
		OpenNICResolverPool				fastest(int num);
		bool							contains(OpenNICResolverPoolItem item);
		bool							contains(QHostAddress item);
		QList<OpenNICResolverPoolItem>&	items()			{return mItems;}
		int								count()			{mItems.count();}
		OpenNICResolverPoolItem			at(int pos)		{return items().at(pos);}
		void							append(OpenNICResolverPoolItem item);
		void							insort(OpenNICResolverPoolItem item);
		int								indexOf(OpenNICResolverPoolItem item);
		int								indexOf(QHostAddress hostAddress);
		QStringList						toStringList();
	public slots:
		void							result(QHostAddress hostAddress, int latency, int faultCode, QString fault);
		void							sort();
	private:
		void							swap(int a,int b);
		QList<OpenNICResolverPoolItem>	mItems;
};


#endif // OPENNICRESOLVERPOOL_H
