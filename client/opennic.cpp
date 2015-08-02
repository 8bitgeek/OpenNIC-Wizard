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
#include "opennic.h"
#include "ui_opennic.h"
#include "ui_settings.h"
#include "opennicqueryhistorydialog.h"
#include "opennicpacket.h"

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
#include <QProgressBar>

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
, ui(new Ui::OpenNICSettings)
, mRefreshTimer(0)
, mPacketState(0)
, mPacketLength(0)
, mTcpListenPort(0)
, mInitialized(false)
, mBalloonIcon(QSystemTrayIcon::Information)
, mActiveState(false)
, mScoreRulesTextChanged(false)
{
	ui->setupUi(this);
	createActions();
	createTrayIcon();
	QObject::connect(this,SIGNAL(accepted()),this,SLOT(writeSettings()));
	mTcpSocket.close();
	mBalloonStatus = tr("Initializing...");
	mRefreshTimer = startTimer(DEFAULT_REFRESH);
	mLocalNet = new OpenNICNet(&mTcpSocket);
	QObject::connect(mLocalNet,SIGNAL(dataReady(OpenNICNet*)),this,SLOT(dataReady(OpenNICNet*)));
	QObject::connect(&mTcpSocket,SIGNAL(connected()),this,SLOT(tcpConnected()));
	QObject::connect(&mTcpSocket,SIGNAL(disconnected()),this,SLOT(tcpDisconnected()));
	QObject::connect(&mTcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(tcpError(QAbstractSocket::SocketError)));
	QObject::connect(&mTcpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(tcpStateChanged(QAbstractSocket::SocketState)));
	QObject::connect(&mTcpSocket,SIGNAL(hostFound()),this,SLOT(tcpHostFound()));
	QObject::connect(this,SIGNAL(accepted()),this,SLOT(update()));
	QObject::connect(ui->saveT1List,SIGNAL(clicked()),this,SLOT(updateT1List()));
	QObject::connect(ui->buttonBox,SIGNAL(clicked(QAbstractButton*)),this,SLOT(clicked(QAbstractButton*)));
	QObject::connect(ui->refreshNow,SIGNAL(clicked()),this,SLOT(updateDNS()));
	QObject::connect(ui->tabs,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
	QObject::connect(ui->saveDomains,SIGNAL(clicked()),this,SLOT(updateDomains()));
	QObject::connect(ui->resolverPoolTable,SIGNAL(cellClicked(int,int)),this,SLOT(cellClicked(int,int)));
	QObject::connect(ui->resolverPoolTable,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(cellDoubleClicked(int,int)));
	QObject::connect(ui->scoreRuleEditor,SIGNAL(textChanged()),this,SLOT(scoreRuleEditorTextChanged()));
	setDisabledState();
}

OpenNIC::~OpenNIC()
{
	delete ui;
	for(int n=0; n < mHistoryDialogs.count(); n++)
	{
		delete mHistoryDialogs.at(n);
	}
	mHistoryDialogs.clear();
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
			showBalloonMessage( tr( "OpenNIC Resolvers" ), ui->cache->toPlainText() );
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
	if ( ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
	{
		update();
	}
}

/**
  * @brief the current tab changed
  */
void OpenNIC::tabChanged(int tab)
{
	QDialogButtonBox* buttonBox = ui->buttonBox;
	QList<QAbstractButton *> buttons = buttonBox->buttons();
	for(int n=0; n < buttons.count(); n++)
	{
		QAbstractButton* button = buttons.at(n);
		if (buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole || buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
		{
			if ( tab == 1 || tab == 5 )
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
	QSettings settings(QSettings::IniFormat, QSettings::SystemScope, "OpenNIC", "OpenNICClient");
}

/**
  * @brief Store the settings.
  */
void OpenNIC::writeSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::SystemScope, "OpenNIC", "OpenNICClient");
}

/**
  * @brief convert a cell click to an IP address
  */
QString OpenNIC::cellClickToAddress(int row, int /* col */)
{
	QTableWidget* table = ui->resolverPoolTable;
	QTableWidgetItem* item = table->item(row,3);
	return item->text();
}

void OpenNIC::cellClicked ( int row, int col )
{
	OpenNICQueryHistoryDialog* dialog = new OpenNICQueryHistoryDialog(mLocalNet,cellClickToAddress(row,col));
	mHistoryDialogs.append(dialog);
	QObject::connect(dialog,SIGNAL(closing(OpenNICQueryHistoryDialog*)),this,SLOT(closing(OpenNICQueryHistoryDialog*)));
	dialog->show();
}

void OpenNIC::closing(OpenNICQueryHistoryDialog* dialog)
{
	int idx = mHistoryDialogs.indexOf(dialog);
	if ( idx >=0 )
	{
		mHistoryDialogs.takeAt(idx);
		dialog->deleteLater();
	}
}

void OpenNIC::cellDoubleClicked ( int row, int col )
{
	cellClicked(row,col);
}

void OpenNIC::scoreRuleEditorTextChanged()
{
	mScoreRulesTextChanged=true;
}

/**
  * @brief determin the minimum, maximum, and average scores
  * @return the average score
  */
double OpenNIC::scoreMinMax(QStringList& resolverPool, double& min, double& max)
{
	if ( resolverPool.count() > 0 )
	{
		double total=0;
		min=0.0;
		max=0.0;
		/* @brief <status>;<score>;<kind>;<hostAddress>; */
		for(int row=0; row < resolverPool.count(); row++ )
		{
			QStringList elements = resolverPool.at(row).split(";");
			double score = elements.at(1).toDouble();
			total += score;
			if (row==0)
			{
				min = max = score;
			}
			else
			{
				if (score > max) max = score;
				if (score < min) min = score;
			}
		}
		return total/resolverPool.count();
	}
	return 0.0;
}

/**
  * @brief Updates the resolver pool display.
  */
void OpenNIC::updateResolverPool(QStringList resolverPool)
{
	/* @brief <status>;<score>;<kind>;<hostAddress>; */

	QTableWidget* table = ui->resolverPoolTable;
	double minScore,maxScore;
	scoreMinMax(resolverPool, minScore, maxScore);
	table->setRowCount(resolverPool.count());
	for(int row=0; row < resolverPool.count(); row++ )
	{
		QStringList elements = resolverPool.at(row).split(";");
		QString ip		= elements.at(3);
		QString kind	= elements.at(2);
		QString score	= elements.at(1);
		QString status	= elements.at(0);
		QString statusIcon;
		if (status == "R")
		{
			status = tr("FAIL");
			statusIcon = ":/images/status-red.png";
		}
		else if (status == "G")
		{
			status = tr("OK");
			statusIcon = ":/images/status-green.png";
		}
		else if (status == "Y")
		{
			status = tr("WARN");
			statusIcon = ":/images/status-yellow.png";
		}
		table->setItem(row,3,new QTableWidgetItem(ip));
		table->setItem(row,2,new QTableWidgetItem(kind));
#if 0
		table->setItem(row,1,new QTableWidgetItem(score));
#else
		QProgressBar* bar = new QProgressBar();
		bar->setRange((int)(minScore*1000.0),(int)(maxScore*1000.0));
		bar->setValue((int)(score.toDouble()*1000.0));
		table->setCellWidget(row,1,bar);
#endif
		table->setItem(row,0,new QTableWidgetItem(QIcon(statusIcon),status));
	}
	table->resizeColumnsToContents();
	table->resizeRowsToContents();
	table->setSortingEnabled(true);
	table->sortByColumn(1,Qt::DescendingOrder);
}

/**
  * @brief Initialize variables. Do this once after connecting with the server to initialize all the values
  * @brief concerned with cerver variables.
*/
void OpenNIC::pollAllKeys()
{
	if (mLocalNet->isLive() )
	{
		QStringList keys;
		keys << OpenNICPacket::tcp_listen_port;
		keys << OpenNICPacket::refresh_timer_period;
		keys << OpenNICPacket::resolver_cache_size;
		keys << OpenNICPacket::resolver_cache;
		keys << OpenNICPacket::resolver_pool;
		keys << OpenNICPacket::bootstrap_t1_list;
		keys << OpenNICPacket::bootstrap_domains;
		keys << OpenNICPacket::system_text;
		keys << OpenNICPacket::journal_text;
		keys << OpenNICPacket::score_rules;
		keys << OpenNICPacket::score_internal;
		mLocalNet->txPacket().set(OpenNICPacket::poll_keys,keys);
		mLocalNet->send(true);
	}
}

/**
  * @brief Poll for periodic keys
  */
void OpenNIC::pollPeriodicKeys()
{
	if (mLocalNet->isLive() )
	{
		QStringList keys;
		keys << OpenNICPacket::resolver_pool;
		keys << OpenNICPacket::resolver_cache;
		keys << OpenNICPacket::system_text;
		keys << OpenNICPacket::journal_text;
		mLocalNet->txPacket().set(OpenNICPacket::poll_keys,keys);
		mLocalNet->send(true);
	}
}

/**
  * @brief get here when receiver data is ready...
  * @param net the network connection
  */
void OpenNIC::dataReady(OpenNICNet* net)
{
	QMapIterator<QString, QVariant>i(net->rxPacket().data());
	mBalloonStatus.clear();
	while (i.hasNext())
	{
		i.next();
		QString key = i.key();
		QVariant value = i.value();
		if ( key == OpenNICPacket::tcp_listen_port )					mTcpListenPort			=	value.toInt();
		else if ( key == OpenNICPacket::resolver_cache )
		{
			QStringList serverResolverCache = value.toStringList();
			QStringList localResolverCache;
			localResolverCache.clear();
			for(int n=0;n < serverResolverCache.count(); n++)
			{
				QStringList parts = serverResolverCache.at(n).split(";");
				localResolverCache.append(parts.at(3));
			}
			QString newText = localResolverCache.join("\n");
			QString currentText = ui->cache->toPlainText();
			if ( newText != currentText )
			{
				ui->cache->setPlainText(localResolverCache.join("\n"));
			}
		}
		else if ( key == OpenNICPacket::refresh_timer_period )
		{
			if ( ui->refreshRate->value() !=  value.toInt() )
			{
				ui->refreshRate->setValue(value.toInt());
			}
		}
		else if ( key == OpenNICPacket::resolver_cache_size )
		{
			if ( ui->resolverCount->value() != value.toInt() )
			{
				ui->resolverCount->setValue(value.toInt());
			}
		}
		else if ( key == OpenNICPacket::bootstrap_t1_list )
		{
			QString currentText = ui->t1List->toPlainText();
			QString newText = value.toStringList().join("\n");
			if ( currentText != newText )
			{
				ui->t1List->setPlainText(newText);
			}
		}
		else if ( key == OpenNICPacket::bootstrap_domains )
		{
			QString currentText = ui->domainList->toPlainText();
			QString newText = value.toStringList().join("\n");
			if ( currentText != newText )
			{
				ui->domainList->setPlainText(newText);
			}
		}
		else if ( key == OpenNICPacket::resolver_pool )				updateResolverPool(value.toStringList());
		else if ( key == OpenNICPacket::system_text )				ui->systemText->setPlainText(value.toString());
		else if ( key == OpenNICPacket::journal_text )
		{
			QStringList journalText = value.toStringList();
			QListWidget* journal = ui->journalList;
			while(journal->count()>200)
			{
				QListWidgetItem* item = journal->takeItem(0);
				if ( item != NULL )
				{
					delete item;
				}
			}
			journal->addItems(journalText);
		}
		else if ( key == OpenNICPacket::async_message && !value.toString().isEmpty() )
		{
			QMessageBox::information(this,tr("Sevice Message"),value.toString());
		}
		else if ( key == OpenNICPacket::score_rules && !value.toString().isEmpty() )
		{
			ui->scoreRuleEditor->setPlainText(value.toString());
		}
		else if ( key == OpenNICPacket::score_internal )
		{
			ui->useBuiltInScoreRule->setChecked(value.toBool());
		}
	}
}

/**
  * @brief Update the service
  */
void OpenNIC::update()
{
	mLocalNet->txPacket().set(OpenNICPacket::refresh_timer_period,	ui->refreshRate->value());
	mLocalNet->txPacket().set(OpenNICPacket::resolver_cache_size,	ui->resolverCount->value());
	mLocalNet->txPacket().set(OpenNICPacket::score_rules,			ui->scoreRuleEditor->toPlainText());
	mLocalNet->txPacket().set(OpenNICPacket::score_internal,		ui->useBuiltInScoreRule->isChecked());
	mLocalNet->send(true);
}

/**
  * @brief Update the T1 list on the service side.
  */
void OpenNIC::updateT1List()
{
	if ( QMessageBox::question(this,tr("Confirm"),tr("Are you sure you wish to overwrite the T1 bootstrap file?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes)
	{
		mLocalNet->txPacket().set(OpenNICPacket::bootstrap_t1_list,	ui->t1List->toPlainText().split("\n"));
		mLocalNet->send(true);
	}
}

/**
  * @brief Update the domains list on the service side.
  */
void OpenNIC::updateDomains()
{
	if ( QMessageBox::question(this,tr("Confirm"),tr("Are you sure you wish to overwrite the domains bootstrap file?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes)
	{
		mLocalNet->txPacket().set(OpenNICPacket::bootstrap_domains,	ui->domainList->toPlainText().split("\n"));
		mLocalNet->send(true);
	}
}

/**
  * @brief Update the DNS
  */
void OpenNIC::updateDNS()
{
	QDataStream stream(&mTcpSocket);
	QMap<QString,QVariant> clientPacket;
	clientPacket.insert(OpenNICPacket::update_dns,"");
	stream << clientPacket;
	mTcpSocket.flush();
}

/**
  * @brief Contact the service and perform a bi-directional update.
  */
void OpenNIC::connectToService()
{
	if ( !mLocalNet->isLive() )
	{
		mTcpSocket.close();
		QHostAddress localhost(QHostAddress::LocalHost);
		mTcpSocket.connectToHost(localhost,19803,QIODevice::ReadWrite);
	}
}


/**
  * @brief Make the applet look enabled (active/connected).
  */
void OpenNIC::setEnabledState()
{
	mActiveState=true;
	ui->refreshNow->setEnabled(true);
	ui->buttonBox->setEnabled(true);
	ui->resolverPool->setEnabled(true);
	ui->scoreRuleEditor->setEnabled(true);
}

/**
  * @brief Make the applet look disabled (inactive/unconnected)
  */
void OpenNIC::setDisabledState()
{
	mActiveState=false;
	ui->refreshNow->setEnabled(false);
	ui->buttonBox->setEnabled(false);
	ui->resolverPool->setEnabled(false);
	ui->scoreRuleEditor->setEnabled(false);
}


void OpenNIC::tcpConnected()
{
	mBalloonStatus=tr("Connecting...");
	setEnabledState();
	pollAllKeys();
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
		setDisabledState();
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
                       "Copyright (C) 2012-2015 Mike Sharkey <mike@8bitgeek.net>\n"
                       "\n"
                       "This program is free software; you can redistribute it and/or modify\n"
                       "it under the terms of the GNU General Public License as published by\n"
                       "the Free Software Foundation; either version 2 of the License, or\n"
                       "(at your option) any later version.\n"
                       "\n"
                       "This program is distributed in the hope that it will be useful,\n"
                       "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                       "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                       "GNU General Public License for more details.\n"
                       "\n"
                       "You should have received a copy of the GNU General Public License along\n"
                       "with this program; if not, write to the Free Software Foundation, Inc.,\n"
                       "51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n"
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
		pollPeriodicKeys();
	}
	else
	{
		inherited::timerEvent(e);
	}
}



