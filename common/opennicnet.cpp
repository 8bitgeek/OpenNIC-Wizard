/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
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

