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
{
	uiSettings->setupUi(this);
	createActions();
	createTrayIcon();
	mStartTimer = startTimer(1000);
	mUpdateResolverPoolTimer = startTimer(1000*5);
	hide();
	QObject::connect(this,SIGNAL(accepted()),this,SLOT(writeSettings()));
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

QStringList OpenNIC::textToStringList(QString text)
{
	QStringList rc;
	rc = text.split('\n');
	return rc;
}

QString OpenNIC::stringListToText(QStringList list)
{
	QString rc;
	rc = list.join("\n");
	return rc;
}

/**
  * @brief Fetch the settings.
  */
void OpenNIC::readSettings()
{
	QStringList t1list;

	QSettings settings("OpenNIC", "OpenNIC Setup");
	uiSettings->cache->setPlainText(stringListToText(settings.value("cache").toStringList()));
	uiSettings->refreshRate->setValue(settings.value("refresh",DEFAULT_REFRESH).toInt());
	uiSettings->resolverCount->setValue(settings.value("resolvers",DEFAULT_RESOLVERS).toInt());

	uiSettings->t1List->setPlainText(stringListToText(settings.value("t1list",resolver().defaultT1List()).toStringList()));
	uiSettings->t1Count->setValue(settings.value("t1count",DEFAULT_T1_RESOLVERS).toInt());
	uiSettings->t1Random->setChecked(settings.value("t1random",DEFAULT_T1_RANDOM).toBool());
}

/**
  * @brief Store the settings.
  */
void OpenNIC::writeSettings()
{
	QSettings settings("OpenNIC", "OpenNIC Setup");
	settings.setValue("cache",		textToStringList(uiSettings->cache->toPlainText()));
	settings.setValue("refresh",	uiSettings->refreshRate->value());
	settings.setValue("resolvers",	uiSettings->resolverCount->value());

	settings.setValue("t1list",		textToStringList(uiSettings->t1List->toPlainText()));
	settings.setValue("t1count",	uiSettings->t1Count->value());
	settings.setValue("t1random",	uiSettings->t1Random->isChecked());

	killTimer(mRefreshTimer);
	mRefreshTimer = startTimer((uiSettings->refreshRate->value()*60)*1000);

}

void OpenNIC::settings()
{
	showNormal();
}

/**
  * @brief Updates the resolver pool display.
  */
void OpenNIC::updateResolverPool()
{
	int row=0;
	QTableWidget* table = uiSettings->resolverPoolTable;
	QMultiMap<quint64,QString> resolverPool = resolver().getResolverPool();
	table->setRowCount(resolverPool.count());
	QMutableMapIterator<quint64,QString>i(resolverPool);
	while (i.hasNext())
	{
		i.next();
		QString resolver = i.value();
		QString latency(QString::number(i.key()));

		table->setItem(row,0,new QTableWidgetItem(latency));
		table->setItem(row,1,new QTableWidgetItem(resolver));
		++row;
	}
	table->resizeColumnsToContents();
	table->resizeRowsToContents();
}

/**
  * @brief Perform the update function. Fetch DNS candidates, test for which to apply, and apply them.
  */
QString OpenNIC::initializeDNS()
{
	QEventLoop loop;
	QString rc;
	QStringList ips = textToStringList(uiSettings->t1List->toPlainText());
	int resolverCount = uiSettings->t1Count->value() < ips.count() ? uiSettings->t1Count->value() : ips.count();
	QProgressDialog progress("Initializing...", "Cancel", 0, resolverCount, this);
	progress.show();
	for(int n=0; n < resolverCount; n++)
	{
		loop.processEvents();
		rc += tr("Using: ") + ips[n];
		rc += resolver().addResolver(ips[n],n+1) + "\n";
		progress.setValue(n);
		if (progress.wasCanceled())
			break;
	}
	progress.setValue(resolverCount);
	return rc;
}

/**
  * @brief Perform the update function. Fetch DNS candidates, test for which to apply, and apply them.
  */
QString OpenNIC::updateDNS()
{
	QEventLoop loop;
	QString rc;
	QStringList ips = resolver().getResolvers();
	int resolverCount = uiSettings->resolverCount->value() < ips.count() ? uiSettings->resolverCount->value() : ips.count();
	uiSettings->cache->clear();
	for(int n=0; n < resolverCount; n++)
	{
		QString ip = ips[n].trimmed();
		uiSettings->cache->appendPlainText(ip);
		loop.processEvents();
		rc += tr("Using: ") + ip;
		rc += resolver().addResolver(ip,n+1);
	}
	return rc;
}

void OpenNIC::about()
{
	QMessageBox::about(this,tr( "About OpenNIC" ), QString("OpenNIC Version ")+QString(VERSION_STRING)+
						"Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>\n"
						"\n"
						"\"THE BEER-WARE LICENSE\" (Revision 42):\n"
						"Mike Sharkey wrote this file. As long as you retain this notice you\n"
						"can do whatever you want with this stuff. If we meet some day, and you think\n"
						"this stuff is worth it, you can buy me a beer in return.\n"
					   );
}

/**
  * @brief get here on timed events.
  */
void OpenNIC::timerEvent(QTimerEvent* e)
{
	if ( e->timerId() == mStartTimer)
	{
		/* get here just once just after startup */
		readSettings();
		uiSettings->logText->append(initializeDNS());
		uiSettings->logText->append(resolver().getSettingsText().trimmed());
		killTimer(mStartTimer);									/* don't need start timer any more */
		mStartTimer=-1;
		mRefreshTimer = startTimer(1000);						/* run the update function soon */
	}
	else if ( e->timerId() == mRefreshTimer )
	{
		/* get here regularly... */
		readSettings();
		uiSettings->logText->clear();
		uiSettings->logText->append(updateDNS());
		uiSettings->logText->append(resolver().getSettingsText().trimmed());
		/* in case we got here on the short timer, extend it to the settings value... */
		killTimer(mRefreshTimer);
		mRefreshTimer = startTimer((uiSettings->refreshRate->value()*60)*1000);
		mTrayIcon->show();
	}
	else if ( e->timerId() == mUpdateResolverPoolTimer )
	{
		updateResolverPool();
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


