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
#define DEFAULT_CLIENT_TIMEOUT 2

OpenNICSession::OpenNICSession(QTcpSocket* socket, OpenNICServer *server)
: QThread(server)
, mSocket(socket)
, mServer(server)
{
    QObject::connect(mSocket,SIGNAL(readyRead()),this,SLOT(readyRead()));
    QObject::connect(mSocket,SIGNAL(disconnected()),this,SLOT(disconnected()));
    mTimer = startTimer(1000);
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
	mPacket = packet;
}

/**
  * @brief data is available, process it...
  */
void OpenNICSession::readyRead()
{
	QEventLoop loop;
	QDateTime timeout;
	QMap<QString,QVariant> clientPacket;
	QDataStream stream(mSocket);
	timeout = QDateTime::currentDateTime().addSecs(DEFAULT_CLIENT_TIMEOUT);
	while(mSocket->isValid() && !mSocket->bytesAvailable() && QDateTime::currentDateTime() < timeout)
	{
		loop.processEvents();
	}
	while ( mSocket->bytesAvailable() )
	{
		clientPacket.clear();
		stream >> clientPacket;
		if ( !clientPacket.empty() )
		{
			stream << mPacket;
			mSocket->flush();
			timeout = QDateTime::currentDateTime().addSecs(DEFAULT_CLIENT_TIMEOUT);
			while(mSocket->isValid() && mSocket->bytesToWrite()>0 && QDateTime::currentDateTime() < timeout )
			{
				loop.processEvents();
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


void OpenNICSession::timerEvent(QTimerEvent *e)
{
	if ( e->timerId() == mTimer )
	{
        if ( mSocket->isValid() )
        {
            if ( mSocket->bytesAvailable())
            {
                readyRead();
            }
        }
        else
        {
			quit();
		}
	}
}
