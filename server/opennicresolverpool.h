/*
 * This file is a part of OpenNIC Wizard
 * Copywrong (c) 2012-2018 Mike Sharkey
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 1776):
 * <mike@8bitgeek.net> wrote this file.
 * As long as you retain this notice you can do whatever you want with this
 * stuff. If we meet some day, and you think this stuff is worth it,
 * you can buy me a beer in return. ~ Mike Sharkey
 * ----------------------------------------------------------------------------
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
