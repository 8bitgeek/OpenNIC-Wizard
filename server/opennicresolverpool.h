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
		bool							contains(OpenNICResolverPoolItem& item);
		bool							contains(QHostAddress item);
		QList<OpenNICResolverPoolItem>&	items()			{return mItems;}
		int								count()			{return mItems.count();}
		OpenNICResolverPoolItem&		at(int pos)		{return mItems[pos];}
		void							append(OpenNICResolverPoolItem item);
		void							insort(OpenNICResolverPoolItem item);
		int								indexOf(OpenNICResolverPoolItem& item);
		int								indexOf(QHostAddress hostAddress);
		QStringList&					toStringList();
		OpenNICResolverPool&			fromStringList(const QStringList strings);
		OpenNICResolverPool&			operator<<(const QStringList& strings);
	public slots:
		void							setMaxHistoryDepth(int maxHistoryDepth);
		void							setActive(bool active);
		void							randomize();
		void							sort();
		void							clear();
	private:
		void							swap(int a,int b);
		QList<OpenNICResolverPoolItem>	mItems;
		QStringList						mStringList;
};


#endif // OPENNICRESOLVERPOOL_H
