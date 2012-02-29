/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennic.h"
#include "ui_opennic.h"
#include "ui_settings.h"

#include <QObject>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QEventLoop>
#include <QProgressDialog>
#include <QByteArray>
#include <QSettings>
#include <QSpinBox>
#include <QProgressDialog>
#include <QCursor>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMultiMap>
#include <QMap>
#include <QVariant>
#include <QHostAddress>
#include <QDataStream>
#include <QDateTime>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTcpSocket>
#include <QIODevice>

#define DEFAULT_RESOLVERS					3
#define DEFAULT_T1_RESOLVERS				3
#define DEFAULT_T1_RANDOM					true
#define DEFAULT_SERVER_TIMEOUT				2 /* seconds */
#define DEFAULT_REFRESH						(15*1000)
#define	FAST_REFRESH						(5*1000)

#define inherited QDialog

OpenNIC::OpenNIC(QWidget *parent)
: inherited(parent)
, uiSettings(new Ui::OpenNICSettings)
, mInitialized(false)
{
	uiSettings->setupUi(this);
	createActions();
	createTrayIcon();
	QObject::connect(this,SIGNAL(accepted()),this,SLOT(writeSettings()));
	mTcpSocket.close();
#if defined Q_OS_UNIX
	show();
#endif
	mBalloonStatus = tr("Initializing...");
	mRefreshTimer = startTimer(DEFAULT_REFRESH);
}

OpenNIC::~OpenNIC()
{
	delete uiSettings;
}

void OpenNIC::slowRefresh()
{
	killTimer(mRefreshTimer);
	mRefreshTimer = startTimer(DEFAULT_REFRESH);
}

void OpenNIC::fastRefresh()
{
	killTimer(mRefreshTimer);
	mRefreshTimer = startTimer(FAST_REFRESH);
}

void OpenNIC::settings()
{
	setVisible(true);
	showNormal();
	fastRefresh();
}

void OpenNIC::closeEvent(QCloseEvent *event)
{
	hide();
	event->ignore();
	slowRefresh();
}

void OpenNIC::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::MiddleClick:
	case QSystemTrayIcon::Trigger:
	{
		if ( mBalloonStatus.isEmpty() )
		{
			showBalloonMessage( tr( "OpenNIC Resolvers" ), uiSettings->cache->toPlainText() );
		}
		else
		{
			showBalloonMessage( tr( "Status" ), mBalloonStatus );
		}
	}
	break;
	case QSystemTrayIcon::DoubleClick:
	{
		settings();
	}
	break;
	default:
	break;
	}
}

void OpenNIC::showBalloonMessage(QString title, QString body)
{
   QSystemTrayIcon::MessageIcon icon;
   mTrayIcon->showMessage(title, body, icon, 5 * 1000);
}

void OpenNIC::createTrayIcon()
{

	mTrayIconMenu = new QMenu(this);
	mTrayIconMenu->addAction(mActionSettings);
	mTrayIconMenu->addAction(mActionAbout);
	mTrayIconMenu->addSeparator();
	mTrayIconMenu->addAction(mActionQuit);

	mTrayIcon = new QSystemTrayIcon(this);
	mTrayIcon->setContextMenu(mTrayIconMenu);

	mTrayIcon->setIcon(QIcon( ":/images/opennic.png" ));
	setWindowIcon( QIcon( ":/images/opennic.png" ) );
	setWindowTitle( tr("OpenNIC Wizard") );
	QObject::connect(mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
	QObject::connect(&mTcpSocket,SIGNAL(connected()),this,SLOT(tcpConnected()));
	QObject::connect(&mTcpSocket,SIGNAL(disconnected()),this,SLOT(tcpDisconnected()));
	QObject::connect(&mTcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(tcpError(QAbstractSocket::SocketError)));
	QObject::connect(&mTcpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(tcpStateChanged(QAbstractSocket::SocketState)));
	QObject::connect(&mTcpSocket,SIGNAL(hostFound()),this,SLOT(tcpHostFound()));
	QObject::connect(&mTcpSocket,SIGNAL(readyRead()),this,SLOT(readyRead()));
	mTrayIcon->show();
}

void OpenNIC::createActions()
{
	mActionSettings = new QAction(tr("&Settings..."), this);
	QObject::connect(mActionSettings,SIGNAL(triggered()),this,SLOT(settings()));

	mActionAbout = new QAction(tr("&About..."), this);
	QObject::connect(mActionAbout,SIGNAL(triggered()),this,SLOT(about()));

	mActionQuit = new QAction(tr("&Quit"), this);
	QObject::connect(mActionQuit,SIGNAL(triggered()),this,SIGNAL(quit()));

}

/**
  * @brief Fetch the settings.
  */
void OpenNIC::readSettings()
{
	QSettings settings("OpenNIC", "OpenNICClient");
}

/**
  * @brief Store the settings.
  */
void OpenNIC::writeSettings()
{
	QSettings settings("OpenNIC", "OpenNICClient");
}

/**
  * @brief Updates the resolver pool display.
  */
void OpenNIC::updateResolverPool(QStringList resolverPool)
{
	/* server string  <hostAddress>;<avgLatency>;<testCount>;<replyCount>;<lastReply>;<lastTimeout>;<lastFault>;<kind>; */

	QTableWidget* table = uiSettings->resolverPoolTable;
	table->setRowCount(resolverPool.count());
	for(int row=0; row < resolverPool.count(); row++ )
	{
		QStringList resolverData = resolverPool.at(row).split(";");
		QString ip = resolverData.at(0);
		QString latency = resolverData.at(1);
		QString testCount = resolverData.at(2);
		QString replyCount = resolverData.at(3);
		QString timeoutCount = resolverData.at(4);
		QString lastReply = resolverData.at(5);
		QString lastTimeout = resolverData.at(6);
		QString status = resolverData.at(7);
		QString kind = resolverData.at(8);
		table->setItem(row,0,new QTableWidgetItem(kind));
		table->setItem(row,1,new QTableWidgetItem(ip));
		table->setItem(row,2,new QTableWidgetItem(latency));
		table->setItem(row,3,new QTableWidgetItem(status));
		table->setItem(row,4,new QTableWidgetItem(testCount));
		table->setItem(row,5,new QTableWidgetItem(replyCount));
		table->setItem(row,6,new QTableWidgetItem(timeoutCount));
		table->setItem(row,7,new QTableWidgetItem(lastReply));
		table->setItem(row,8,new QTableWidgetItem(lastTimeout));
	}
	table->resizeColumnsToContents();
	table->resizeRowsToContents();
	table->setSortingEnabled(true);
}

void OpenNIC::mapServerReply(QMap<QString,QVariant>& map)
{
	QMapIterator<QString, QVariant>i(map);
	while (i.hasNext())
	{
		i.next();
		QString key = i.key();
		QVariant value = i.value();
		if ( key == "tcp_listen_port" )					mTcpListenPort			=	value.toInt();
		else if ( key == "log_file" )					mLogFile				=	value.toString();
		else if ( key == "resolver_cache" )
		{
			QStringList serverResolverCache = value.toStringList();
			mResolverCache.clear();
			for(int n=0;n < serverResolverCache.count(); n++)
			{
				QStringList parts = serverResolverCache.at(n).split(";");
				mResolverCache.append(parts.at(0));
			}
		}
		else if ( key == "refresh_timer_period" )		mRefreshTimerPeriod		=	value.toInt();
		else if ( key == "resolver_cache_size" )		mResolverCacheSize		=	value.toInt();
		else if ( key == "bootstrap_t1_list" )			mBootstrapT1List		=	value.toStringList();
		else if ( key == "resolver_pool" )				updateResolverPool(value.toStringList());
		else if ( key == "system_text" )				uiSettings->systemText->setPlainText(value.toString());
		else if ( key == "journal_text" )
		{
			QStringList journalText = value.toStringList();
			QListWidget* journal = uiSettings->journalList;
			while(journal->count()>150)
			{
				QListWidgetItem* item = journal->takeItem(0);
				if ( item != NULL )
				{
					delete item;
				}
			}
			journal->addItems(journalText);
		}
	}
	uiSettings->cache->setPlainText(mResolverCache.join("\n"));
	uiSettings->t1List->setPlainText(mBootstrapT1List.join("\n"));
	uiSettings->refreshRate->setValue(mRefreshTimerPeriod);
	uiSettings->resolverCount->setValue(mResolverCacheSize);
}

/**
  * @brief Map client status
  * @return a map of key/value pairs
  */
QMap<QString,QVariant> OpenNIC::mapClientStatus()
{
	QMap<QString,QVariant> map;
	map.insert("refresh_timer_period",		mRefreshTimerPeriod);
	map.insert("resolver_cache_size",		mResolverCacheSize);
	return map;
}

/**
  * @brief Contact the service and perform a bi-directional update.
  */
void OpenNIC::connectToService()
{
	if ( !mTcpSocket.isValid() || !mTcpSocket.isOpen() )
    {
        mTcpSocket.close();
        QHostAddress localhost(QHostAddress::LocalHost);
        mTcpSocket.connectToHost(localhost,19803,QIODevice::ReadWrite);
    }
	else
	{
		update();
	}
}

/**
  * @brief Update the service
  */
void OpenNIC::update()
{
	QDataStream stream(&mTcpSocket);
	QMap<QString,QVariant> clientPacket;
	if ( !mInitialized )
	{
		clientPacket.insert("initialize",VERSION_STRING); /* something to get a reply */
		mInitialized=true;
	}
	else
	{
		clientPacket = mapClientStatus();
	}
	stream << clientPacket;
	mTcpSocket.flush();
}

/**
  * @brief get here when data is available from the server.
  */
void OpenNIC::readyRead()
{
	QDataStream stream(&mTcpSocket);
	QMap<QString,QVariant> serverPacket;
	mTcpSocket.flush();
	while ( mTcpSocket.bytesAvailable() )
	{
		serverPacket.clear();
		stream >> serverPacket;
		if (!serverPacket.isEmpty() )
		{
			mBalloonStatus="";
			mapServerReply(serverPacket);
		}
	}
	if ( !mTcpSocket.isValid() || !mTcpSocket.isOpen() )
	{
		mBalloonStatus=tr("OpenNIC Service unexpectedly closed");
	}
}

void OpenNIC::tcpConnected()
{
	update();
}

void OpenNIC::tcpDisconnected()
{
	mBalloonStatus=tr("OpenNIC Service closed the connection");
	mTcpSocket.close();
}

void OpenNIC::tcpError(QAbstractSocket::SocketError socketError)
{
	if ( socketError != QAbstractSocket::RemoteHostClosedError )
	{
		mBalloonStatus = tr( "Failed to connect to OpenNIC service. [" ) + QString::number((int)socketError) + "]";
		mTcpSocket.close();
		slowRefresh();
	}
}

void OpenNIC::tcpHostFound()
{
}

void OpenNIC::tcpStateChanged(QAbstractSocket::SocketState socketState)
{
}


void OpenNIC::about()
{
	QMessageBox::about(this,tr( "About OpenNIC" ), QString("OpenNIC Version ")+QString(VERSION_STRING)+
						" Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>\n"
						"\n"
						"\"THE BEER-WARE LICENSE\" (Revision 42):\n"
						"Mike Sharkey wrote this thing. As long as you retain this notice you\n"
						"can do whatever you want with this stuff. If we meet some day, and you think\n"
						"this stuff is worth it, you can buy me a beer in return.\n"
					   );
}

/**
  * @brief get here on timed events.
  */
void OpenNIC::timerEvent(QTimerEvent* e)
{
	if ( e->timerId() == mRefreshTimer )
	{
        connectToService();
		mTrayIcon->show();
	}
	else
	{
		inherited::timerEvent(e);
	}
}



