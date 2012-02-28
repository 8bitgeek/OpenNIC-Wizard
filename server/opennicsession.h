/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICSESSION_H
#define OPENNICSESSION_H

#include <QThread>
#include <QTcpSocket>

class OpenNICServer;
class OpenNICSession : public QThread
{
    Q_OBJECT
    public:
        OpenNICSession(QTcpSocket* socket, OpenNICServer* server);
        virtual ~OpenNICSession();
        OpenNICServer*  server()    {return mServer;}
        QTcpSocket*     socket()    {return mSocket;}
    protected slots:
        virtual void    readyRead();
        virtual void    disconnected();
    protected:
        virtual void    run();
    private:
        QTcpSocket*     mSocket;
        OpenNICServer*  mServer;
};

#endif // OPENNICSESSION_H
