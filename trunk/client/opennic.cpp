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
#include <QTabWidget>
#include <QWidget>

#define DEFAULT_RESOLVERS					3
#define DEFAULT_T1_RESOLVERS				3
#define DEFAULT_T1_RANDOM					true
#define DEFAULT_SERVER_TIMEOUT				2 /* seconds */
#define DEFAULT_REFRESH						(15*1000)
#define	FAST_REFRESH						(5*1000)

#define inherited QDialog





OpenNIC::OpenNIC(QWidget *parent)
: inherited(parent)
, mActionSettings(NULL)
, mActionAbout(NULL)
, mActionQuit(NULL)
, mTrayIcon(NULL)
, mTrayIconMenu(NULL)
, uiSettings(new Ui::OpenNICSettings)
, mRefreshTimer(0)
, mPacketState(0)
, mPacketLength(0)
, mTcpListenPort(0)
, mInitialized(false)
, mBalloonIcon(QSystemTrayIcon::Information)

{
	uiSettings->setupUi(this);
	createActions();
	createTrayIcon();
	QObject::connect(this,SIGNAL(accepted()),this,SLOT(writeSettings()));
	mTcpSocket.close();
	mBalloonStatus = tr("Initializing...");
	mRefreshTimer = startTimer(DEFAULT_REFRESH);
	QObject::connect(&mTcpSocket,SIGNAL(connected()),this,SLOT(tcpConnected()));
	QObject::connect(&mTcpSocket,SIGNAL(disconnected()),this,SLOT(tcpDisconnected()));
	QObject::connect(&mTcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(tcpError(QAbstractSocket::SocketError)));
	QObject::connect(&mTcpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(tcpStateChanged(QAbstractSocket::SocketState)));
	QObject::connect(&mTcpSocket,SIGNAL(hostFound()),this,SLOT(tcpHostFound()));
	QObject::connect(&mTcpSocket,SIGNAL(readyRead()),this,SLOT(readyRead()));
	QObject::connect(this,SIGNAL(accepted()),this,SLOT(update()));
	QObject::connect(uiSettings->saveT1List,SIGNAL(clicked()),this,SLOT(updateT1List()));
	QObject::connect(uiSettings->buttonBox,SIGNAL(clicked(QAbstractButton*)),this,SLOT(clicked(QAbstractButton*)));
	QObject::connect(uiSettings->refreshNow,SIGNAL(clicked()),this,SLOT(updateDNS()));
	QObject::connect(uiSettings->tabs,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
	QObject::connect(uiSettings->saveDomains,SIGNAL(clicked()),this,SLOT(updateDomains()));
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
			mBalloonIcon = QSystemTrayIcon::Information;
			showBalloonMessage( tr( "OpenNIC Resolvers" ), uiSettings->cache->toPlainText() );
		}
		else
		{
			mBalloonIcon = QSystemTrayIcon::Warning;
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
	mTrayIcon->showMessage(title, body, mBalloonIcon, 5 * 1000);
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
	mTrayIcon->show();
}

void OpenNIC::createActions()
{
	mActionSettings = new QAction(tr("&Settings..."), this);
	QObject::connect(mActionSettings,SIGNAL(triggered()),this,SLOT(settings()));

	mActionAbout = new QAction(tr("&About..."), this);
	QObject::connect(mActionAbout,SIGNAL(triggered()),this,SLOT(about()));

	mActionQuit = new QAction(tr("&Quit"), this);
	QObject::connect(mActionQuit,SIGNAL(triggered()),this,SLOT(maybeQuit()));

}

/**
  * @brief maybe wuit
  */
void OpenNIC::maybeQuit()
{
	if ( QMessageBox::question(this,tr("Quit OpenNIC Applet"),tr("Do you really wish to quit the OpenNIC Applet?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes )
	{
		emit quit();
	}
}

/**
  * @brief a buttonbox button was clicked.
  */
void OpenNIC::clicked(QAbstractButton* button)
{
	if ( uiSettings->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
	{
		update();
	}
}

/**
  * @brief the current tab changed
  */
void OpenNIC::tabChanged(int tab)
{
	QDialogButtonBox* buttonBox = uiSettings->buttonBox;
	QList<QAbstractButton *> buttons = buttonBox->buttons();
	for(int n=0; n < buttons.count(); n++)
	{
		QAbstractButton* button = buttons.at(n);
		if (buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole || buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
		{
			if ( tab == 1 )
			{
				button->setEnabled(true);
			}
			else
			{
				button->setEnabled(false);
			}
		}
	}
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

void OpenNIC::storeServerPacket(QMap<QString,QVariant>& map)
{
	QMapIterator<QString, QVariant>i(map);
	while (i.hasNext())
	{
		i.next();
		QString key = i.key();
		QVariant value = i.value();
		if ( key == "tcp_listen_port" )					mTcpListenPort			=	value.toInt();
		else if ( key == "resolver_cache" )
		{
			QStringList serverResolverCache = value.toStringList();
			QStringList localResolverCache;
			localResolverCache.clear();
			for(int n=0;n < serverResolverCache.count(); n++)
			{
				QStringList parts = serverResolverCache.at(n).split(";");
				localResolverCache.append(parts.at(0));
			}
			QString newText = localResolverCache.join("\n");
			QString currentText = uiSettings->cache->toPlainText();
			if ( newText != currentText )
			{
				uiSettings->cache->setPlainText(localResolverCache.join("\n"));
			}
		}
		else if ( key == "refresh_timer_period" )
		{
			if ( uiSettings->refreshRate->value() !=  value.toInt() )
			{
				uiSettings->refreshRate->setValue(value.toInt());
			}
		}
		else if ( key == "resolver_cache_size" )
		{
			if ( uiSettings->resolverCount->value() != value.toInt() )
			{
				uiSettings->resolverCount->setValue(value.toInt());
			}
		}
		else if ( key == "bootstrap_t1_list" )
		{
			QString currentText = uiSettings->t1List->toPlainText();
			QString newText = value.toStringList().join("\n");
			if ( currentText != newText )
			{
				uiSettings->t1List->setPlainText(newText);
			}
		}
		else if ( key == "bootstrap_domains" )
		{
			QString currentText = uiSettings->domainList->toPlainText();
			QString newText = value.toStringList().join("\n");
			if ( currentText != newText )
			{
				uiSettings->domainList->setPlainText(newText);
			}
		}
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
		else if ( key == "async_message" && !value.toString().isEmpty() )
		{
			QMessageBox::information(this,tr("Sevice Message"),value.toString());
		}
	}
}

/**
  * @brief Map client status
  * @return a map of key/value pairs
  */
QMap<QString,QVariant> OpenNIC::clientSettingsPacket()
{
	QMap<QString,QVariant> map;
	map.insert("refresh_timer_period", uiSettings->refreshRate->value());
	map.insert("resolver_cache_size", uiSettings->resolverCount->value());
	return map;
}

/**
  * @brief Update the service
  */
void OpenNIC::update()
{
	QDataStream stream(&mTcpSocket);
	QMap<QString,QVariant> clientPacket;
	clientPacket = clientSettingsPacket();
	stream << clientPacket;
	mTcpSocket.flush();
}

/**
  * @brief Update the T1 list on the service side.
  */
void OpenNIC::updateT1List()
{
	if ( QMessageBox::question(this,tr("Confirm"),tr("Are you sure you wish to overwrite the T1 bootstrap file?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes)
	{
		QDataStream stream(&mTcpSocket);
		QMap<QString,QVariant> clientPacket;
		QString t1Text = uiSettings->t1List->toPlainText();
		clientPacket.insert("bootstrap_t1_list",t1Text.split("\n"));
		stream << clientPacket;
		mTcpSocket.flush();
	}
}

/**
  * @brief Update the domains list on the service side.
  */
void OpenNIC::updateDomains()
{
	if ( QMessageBox::question(this,tr("Confirm"),tr("Are you sure you wish to overwrite the domains bootstrap file?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes)
	{
		QDataStream stream(&mTcpSocket);
		QMap<QString,QVariant> clientPacket;
		QString domainsText = uiSettings->domainList->toPlainText();
		clientPacket.insert("bootstrap_domains",domainsText.split("\n"));
		stream << clientPacket;
		mTcpSocket.flush();
	}
}

/**
  * @brief Update the DNS
  */
void OpenNIC::updateDNS()
{
	QDataStream stream(&mTcpSocket);
	QMap<QString,QVariant> clientPacket;
	clientPacket.insert("update_dns","");
	stream << clientPacket;
	mTcpSocket.flush();
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
}

/**
  * @brief get here when data is available from the server.
  */
void OpenNIC::readyRead()
{
	QByteArray chunk;
	QDataStream stream(&mTcpSocket);
	switch(mPacketState)
	{
	case 0:
		mPacketBytes.clear();
		stream >> mPacketLength;
		if (mPacketLength == 0xFFFFFFFF)
		{
			return;
		}
		mPacketState=1;
	case 1:
		chunk = stream.device()->readAll();
		mPacketBytes.append(chunk);
		if (mPacketBytes.length()<(int)mPacketLength)
		{
			return;
		}
		mPacketState=0;
		break;
	}
	QMap<QString,QVariant> serverPacket;
	QDataStream byteStream(&mPacketBytes,QIODevice::ReadOnly);
	byteStream >> serverPacket;
	if (!serverPacket.isEmpty() )
	{
		mBalloonStatus="";
		storeServerPacket(serverPacket);
	}
	if ( !mTcpSocket.isValid() || !mTcpSocket.isOpen() )
	{
		mBalloonStatus=tr("OpenNIC Service unexpectedly closed");
	}
}

void OpenNIC::tcpConnected()
{
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

void OpenNIC::tcpStateChanged(QAbstractSocket::SocketState /* socketState */)
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


