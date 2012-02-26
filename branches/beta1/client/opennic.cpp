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

#define DEFAULT_REFRESH			15
#define DEFAULT_RESOLVERS		3
#define DEFAULT_T1_RESOLVERS	3
#define DEFAULT_T1_RANDOM		true

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
#if 1 /* DEBUG */
	show();
#endif
}

OpenNIC::~OpenNIC()
{
	delete uiSettings;
}

void OpenNIC::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
   switch (reason)
   {
	   case QSystemTrayIcon::MiddleClick:
	   case QSystemTrayIcon::Trigger:
	   showBalloonMessage( tr( "OpenNIC Resolvers" ), uiSettings->cache->toPlainText() );
		   break;
	   case QSystemTrayIcon::DoubleClick:
		   setVisible(true);
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
	setWindowTitle( "OpenNIC Setup" );
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

void OpenNIC::settings()
{
	showNormal();
}

/**
  * @brief Updates the resolver pool display.
  */
void OpenNIC::updateResolverPool(QStringList resolverPool)
{
	QTableWidget* table = uiSettings->resolverPoolTable;
	table->setRowCount(resolverPool.count());
	for(int row=0; row < resolverPool.count(); row++ )
	{
		QStringList resolverData = resolverPool.at(row).split(";");
		QString resolver = resolverData.at(0);
		QString latency = resolverData.at(1);
		table->setItem(row,0,new QTableWidgetItem(latency));
		table->setItem(row,1,new QTableWidgetItem(resolver));

	}
	table->resizeColumnsToContents();
	table->resizeRowsToContents();
}

/**
  * @brief Contact the service and perform a bi-directional update.
  */
void OpenNIC::updateService()
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
		updateService();
		mTrayIcon->show();
	}
	else
	{
		inherited::timerEvent(e);
	}
}


void OpenNIC::closeEvent(QCloseEvent *event)
{
	hide();
	event->ignore();
}


