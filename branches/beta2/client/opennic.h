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
#include <QMap>
#include <QVariant>
#include <QTcpSocket>

#define	VERSION_STRING	"0.2.4"

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
		void					mapServerReply(QMap<QString,QVariant>& map);
		QMap<QString,QVariant>	mapClientStatus();
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
        void                    update();
		void					tcpConnected();
		void					tcpDisconnected();
		void					tcpError(QAbstractSocket::SocketError socketError);
		void					tcpHostFound();
		void					tcpStateChanged(QAbstractSocket::SocketState socketState);

		void					updateResolverPool(QStringList resolverPool);
        void					connectToService();
		void					iconActivated(QSystemTrayIcon::ActivationReason reason);
		void					showBalloonMessage(QString title, QString body);

	private:
		QAction*				mActionSettings;
		QAction*				mActionAbout;
		QAction*				mActionQuit;
		QSystemTrayIcon*		mTrayIcon;
		QMenu*					mTrayIconMenu;
		Ui::OpenNICSettings*	uiSettings;
		int						mRefreshTimer;
		QTcpSocket				mTcpSocket;
        QString                 mBalloonStatus;             /** status message to apply to ballon */
		/** server settings **/
		int						mTcpListenPort;				/** the TCP listen port */
		QString					mLogFile;					/** the log file */
		bool					mInitialized;				/** server variables are initialized? */
		QStringList				mBootstrapT1List;			/** list of bootstrap T1's */
		int						mBootstrapCacheSize;		/** number of T1s to select for boostrap */
		bool					mBootstrapRandomSelect;		/** select bootstrap resolvers randomly */
		QStringList				mResolverCache;				/** most recently selected resolver cache */
		int						mResolverCacheSize;			/** the number of resolvers to keep in the cache (and apply to the O/S) */
		int						mRefreshTimerPeriod;		/** the resolver refresh rate (apply cache to O/S) */
};

#endif // OPENNIC_H
