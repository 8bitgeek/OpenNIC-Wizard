/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicsession.h"
#include "opennicserver.h"

#include <QEventLoop>

#define inherited QThread

OpenNICSession::OpenNICSession(QTcpSocket* socket, OpenNICServer *server)
: inherited(server)
, mSocket(socket)
, mServer(server)
{
	QObject::connect(mSocket,SIGNAL(readyRead()),this,SLOT(readyRead()));
	QObject::connect(mSocket,SIGNAL(disconnected()),this,SLOT(disconnected()));
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
  * @brief receive a packet
  */
void OpenNICSession::packet(QMap<QString,QVariant> packet)
{
	if ( mSocket != NULL && mSocket->isValid() && mSocket->isOpen() )
	{
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
			emit sessionPacket(this,clientPacket);
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

