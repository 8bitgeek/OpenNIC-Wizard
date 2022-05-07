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

#include "opennicsession.h"
#include "opennicserver.h"

#include <QEventLoop>

#define inherited QThread

OpenNICSession::OpenNICSession(QTcpSocket* socket)
: mSocket(socket)
{
	QObject::connect(mSocket,SIGNAL(readyRead()),this,SLOT(readyRead()));
	QObject::connect(mSocket,SIGNAL(disconnected()),this,SLOT(disconnected()));
	mTimer = startTimer(1000*5);
}

OpenNICSession::~OpenNICSession()
{
	mSocket->close();
	delete mSocket;
	mSocket=NULL;
}

void OpenNICSession::run()
{
    exec();
}

/**
  * @brief send a packet
  */
void OpenNICSession::sendPacket()
{
	if ( mSocket != NULL && mSocket->isValid() && mSocket->isOpen() )
	{
		QMap<QString,QVariant> packet;
		OpenNICServer::getPacket(packet);
		QDataStream stream(mSocket);
		stream << packet;						/* pump it out. */
		mSocket->flush();
	}
}

/**
  * @brief data is available, process it...
  */
void OpenNICSession::readyRead()
{
	QMap<QString,QVariant> clientPacket;
	QDataStream stream(mSocket);
	while ( mSocket != NULL && mSocket->isValid() && mSocket->isOpen() && mSocket->bytesAvailable() )
	{
		clientPacket.clear();
		stream >> clientPacket;
		if ( !clientPacket.empty() )
		{
			QMapIterator<QString, QVariant>i(clientPacket);
			while (i.hasNext())
			{
				i.next();
				QString key = i.key();
				QVariant value = i.value();
				if ( key == "refresh_timer_period" )
				{
					OpenNICServer::setRefreshPeriod(value.toInt());
				}
				else if ( key == "resolver_cache_size" )
				{
					OpenNICServer::setResolverCacheSize(value.toInt());
				}
			}
		}
	}
}

/**
  * @brief the socket has been disconnected.
  */
void OpenNICSession::disconnected()
{
    quit();
}

void OpenNICSession::timerEvent(QTimerEvent* e)
{
	if ( e->timerId() == mTimer )
	{
		sendPacket();
	}
}
