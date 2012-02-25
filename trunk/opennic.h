/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNIC_H
#define OPENNIC_H

#include <QMainWindow>
#include <QString>
#include <QTextBrowser>
#include <QStringList>
#include <QTimerEvent>
#include <QCloseEvent>
#include <QDialog>
#include <QSystemTrayIcon>
#include <QAction>

#include "opennicresolver.h"

#define	VERSION_STRING	"0.0.3"

namespace Ui
{
	class OpenNICSettings;
}

class OpenNIC : public QDialog
{
	Q_OBJECT

	public:
		explicit OpenNIC(QWidget *parent = 0);
		virtual ~OpenNIC();

	signals:
		void					quit();

	protected:
		void					updateResolverPool();
		OpenNICResolver&		resolver() {return mResolver;}
		QString					initializeDNS();
		QString					updateDNS();
		QStringList				textToStringList(QString text);
		QString					stringListToText(QStringList list);
		void					createTrayIcon();
		void					createActions();
		virtual void			timerEvent(QTimerEvent* e);
		virtual void			closeEvent(QCloseEvent* e);

	protected slots:
		void					readSettings();
		void					writeSettings();
		void					settings();
		void					about();

	private slots:
		void					iconActivated(QSystemTrayIcon::ActivationReason reason);
		void					showBalloonMessage(QString title, QString body);

	private:
		QAction*				mActionSettings;
		QAction*				mActionAbout;
		QAction*				mActionQuit;
		QSystemTrayIcon*		mTrayIcon;
		QMenu*					mTrayIconMenu;
		Ui::OpenNICSettings*	uiSettings;
		int						mStartTimer;
		int						mRefreshTimer;
		int						mUpdateResolverPoolTimer;
		OpenNICResolver			mResolver;
};

#endif // OPENNIC_H
