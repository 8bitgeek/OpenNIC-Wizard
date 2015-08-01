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
#include <QMap>
#include <QVariant>
#include <QTcpSocket>
#include <QAbstractButton>

#include "opennicqueryhistorydialog.h"
#include "opennicversion.h"
#include "opennicnet.h"

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

	public slots:
		void					connectToService();


	signals:
		void					quit();

	protected:
		QString					cellClickToAddress(int row, int column);
		void					storeServerPacket(QMap<QString,QVariant>& map);
		void					createTrayIcon();
		void					createActions();
		virtual void			timerEvent(QTimerEvent* e);
		virtual void			closeEvent(QCloseEvent* e);

	private slots:
		void					closing(OpenNICQueryHistoryDialog* dialog);
		void					pollAllKeys();
		void					pollPeriodicKeys();
		void					dataReady(OpenNICNet* net);
		void					scoreRuleEditorTextChanged();
		void					cellClicked ( int row, int column );
		void					cellDoubleClicked ( int row, int column );
		void					setEnabledState();
		void					setDisabledState();
		void					tabChanged(int tab);
		void					clicked(QAbstractButton* button);
		void					update();
		void					updateT1List();
		void					updateDomains();
		void					updateDNS();
		void					readSettings();
		void					writeSettings();
		void					settings();
		void					about();
		void					maybeQuit();
		void					fastRefresh();
		void					slowRefresh();
		void					tcpConnected();
		void					tcpDisconnected();
		void					tcpError(QAbstractSocket::SocketError socketError);
		void					tcpHostFound();
		void					tcpStateChanged(QAbstractSocket::SocketState socketState);
		double					scoreMinMax(QStringList& resolverPool, double& min, double& max);
		void					updateResolverPool(QStringList resolverPool);
		void					iconActivated(QSystemTrayIcon::ActivationReason reason);
		void					showBalloonMessage(QString title, QString body);

	private:
		QAction*				mActionSettings;
		QAction*				mActionAbout;
		QAction*				mActionQuit;
		QSystemTrayIcon*		mTrayIcon;
		QMenu*					mTrayIconMenu;
		Ui::OpenNICSettings*	ui;
		int						mRefreshTimer;
		int						mPacketState;				/** 0 expecting, 1 receiving, 2 completed. */
		quint32					mPacketLength;
		int						mTcpListenPort;				/** the TCP listen port */
		bool					mInitialized;				/** server variables are initialized? */
        QString                 mBalloonStatus;             /** status message to apply to ballon */
		QSystemTrayIcon::MessageIcon mBalloonIcon;
		bool					mActiveState;				/** applet if connected and live or disconeced */
		bool					mScoreRulesTextChanged;
		QTcpSocket				mTcpSocket;
		OpenNICNet*				mLocalNet;					/** lochost comm */
		QList<OpenNICQueryHistoryDialog*> mHistoryDialogs;
};

#endif // OPENNIC_H
