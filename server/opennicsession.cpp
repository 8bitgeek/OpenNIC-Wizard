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



OpenNICSession::OpenNICSession(QTcpSocket* socket, OpenNICServer *server)
: QThread(server)
, mSocket(socket)
, mServer(server)
{
    QObject::connect(mSocket,SIGNAL(readyRead()),this,SLOT(readyRead()));
    QObject::connect(mSocket,SIGNAL(disconnected()),this,SLOT(disconnected()));
}

OpenNICSession::~OpenNICSession()
{
    delete mSocket;
    mSocket=NULL;
}

void OpenNICSession::run()
{
    exec();
}

/**
  * @brief data is available, process it...
  */
void OpenNICSession::readyRead()
{
    server()->processMutex().lock();
    server()->process(mSocket);
    server()->processMutex().unlock();
}

/**
  * @brief the socket has been disconnected.
  */
void OpenNICSession::disconnected()
{
    quit();
}

