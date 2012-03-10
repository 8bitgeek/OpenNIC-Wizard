/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICNET_H
#define OPENNICNET_H

#include <QObject>
#include <QTcpSocket>

#include "opennicpacket.h"

class OpenNICNet : public QObject
{
    Q_OBJECT
	public:
		OpenNICNet(QTcpSocket* socket, QObject *parent = 0);
		virtual ~OpenNICNet();
		QTcpSocket*				socket()	{return mSocket;}
		OpenNICPacket&			rxPacket()	{return mRXPacket;}
		OpenNICPacket&			txPacket()	{return mTXPacket;}
		bool					isLive();
	public slots:
		void					clear();
		void					send(bool clear=false);
	signals:
		void					dataReady(OpenNICNet* net);
	private slots:
		void					readyRead();
		void					dataReady();
	private:
		QTcpSocket*				mSocket;
		OpenNICPacket			mRXPacket;
		OpenNICPacket			mTXPacket;
};

#endif // OPENNICNET_H
