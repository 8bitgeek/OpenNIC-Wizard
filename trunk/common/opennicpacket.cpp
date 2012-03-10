/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicpacket.h"

#include <QDataStream>
#include <QByteArray>

/** Keys */
const QString OpenNICPacket::tcp_listen_port		= "tcp_listen_port";		/* the tcp port to listen on */
const QString OpenNICPacket::refresh_timer_period	= "refresh_timer_period";	/* the service refresh timer period */
const QString OpenNICPacket::resolver_cache_size	= "resolver_cache_size";	/* the resolver cache size */
const QString OpenNICPacket::resolver_pool			= "resolver_pool";			/* the resolver pool */
const QString OpenNICPacket::resolver_cache			= "resolver_cache";			/* the resolver cache */
const QString OpenNICPacket::bootstrap_t1_list		= "bootstrap_t1_list";		/* the bootstrap t1 liost */
const QString OpenNICPacket::bootstrap_domains		= "bootstrap_domains";		/* the bootstrap domains */
const QString OpenNICPacket::system_text			= "system_text";			/* the system text */
const QString OpenNICPacket::journal_text			= "journal_text";			/* the journal text */
const QString OpenNICPacket::async_message			= "async_message";			/* an async message */
const QString OpenNICPacket::score_rules			= "score_rules";			/* score rules */
const QString OpenNICPacket::score_internal			= "score_internal";			/* score internal */


OpenNICPacket::OpenNICPacket(QObject *parent)
: QObject(parent)
, mRXState(0)
, mRXLength(0)
{
}

/**
  * @brief set the name value pair of a data record in the packet
  */
void OpenNICPacket::set(QString key, QVariant value)
{
	mData.insert(key,value);
}

/**
  * @return the value from a key
  */
QVariant OpenNICPacket::get(QString key)
{
	QVariant rc;
	if (contains(key))
	{
		rc = mData[key];
	}
	return rc;
}

/**
  * @brief clear the packet state
  */
void OpenNICPacket::clear()
{
	mData.clear();
}

/**
  * @brief receive chunk of data from a socket to this packet
  */
void OpenNICPacket::recv(QTcpSocket* socket)
{
	QByteArray chunk;
	QDataStream stream(socket);
	switch(mRXState)
	{
	case 0:
		mRXData.clear();
		stream >> mRXLength;
		if (mRXLength == 0xFFFFFFFF)
		{
			return;
		}
		mRXState=1;
	case 1:
		chunk = stream.device()->readAll();
		mRXData.append(chunk);
		if (mRXData.length()<(int)mRXLength)
		{
			return;
		}
		mRXState=0;
		break;
	}
	QDataStream dataStream(&mRXData,QIODevice::ReadOnly);
	dataStream >> mData;
	emit dataReady();
}

/**
  * @brief send this packet out a socket.
  */
void OpenNICPacket::send(QTcpSocket* socket)
{
	if ( socket->isOpen() && socket->isValid() )
	{
		QDataStream stream(socket);
		stream << mData;
		socket->flush();
	}
}

