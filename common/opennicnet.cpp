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
#include "opennicnet.h"

OpenNICNet::OpenNICNet(QTcpSocket* socket, QObject *parent)
: QObject(parent)
, mSocket(socket)
{
	QObject::connect(mSocket,SIGNAL(readyRead()),this,SLOT(readyRead()));
	QObject::connect(&mRXPacket,SIGNAL(dataReady()),this,SLOT(dataReady()));
}

OpenNICNet::~OpenNICNet()
{
	if ( mSocket->isOpen() && mSocket->isValid() )
	{
		mSocket->flush();
		mSocket->close();
	}
	delete mSocket;
	mSocket=NULL;
}

/**
  * @brief clear state
  */
void OpenNICNet::clear()
{
	mTXPacket.clear();
	mRXPacket.clear();
}

/**
  * @return the state of the socket
  */
bool OpenNICNet::isLive()
{
	return socket()->isOpen() && socket()->isValid();
}

/**
  * @brief
  */
void OpenNICNet::send(bool clear)
{
	if ( isLive() )
	{
		mTXPacket.send(mSocket);
	}
	if (clear)
	{
		this->clear();
	}
}

/**
  * @brief Get here on data read socket signal
  */
void OpenNICNet::readyRead()
{
	mRXPacket.recv(mSocket);
}

/**
  * @brief Get here when a RX packet is ready
  */
void OpenNICNet::dataReady()
{
	bool tx=false;
	bool rx=false;
	QMapIterator<QString, QVariant>i(rxPacket().data());
	while (i.hasNext())
	{
		i.next();
		if ( i.key() == OpenNICPacket::poll_keys )
		{
			QStringList keys = i.value().toStringList();
			for(int n=0; n < keys.count(); n++)
			{
				QString key = keys[n];
				QVariant value;
				bool valid;
				emit pollKeyValue(key,value,valid);
				if ( valid )
				{
					txPacket().set(key,value);
				}
			}
			tx=true;
		}
		else
		{
			rx=true;
		}
	}
	if ( tx )
	{
		send(true);
	}
	if ( rx )
	{
		emit dataReady(this);
	}
}

