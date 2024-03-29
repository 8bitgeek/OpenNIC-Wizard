/*
 * This file is a part of OpenNIC Wizard
 * Copywrong (c) 2012-2022 Mike Sharkey
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 1776):
 * <mike@8bitgeek.net> wrote this file.
 * As long as you retain this notice you can do whatever you want with this
 * stuff. If we meet some day, and you think this stuff is worth it,
 * you can buy me a beer in return. ~ Mike Sharkey
 * ----------------------------------------------------------------------------
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

		void					enabledChanged(int state);
		void 					interfaceChanged(const QString& interface);

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
