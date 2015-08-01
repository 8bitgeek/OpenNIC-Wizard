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
		void					pollKeyValue(QString& key, QVariant& value, bool& valid);
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
