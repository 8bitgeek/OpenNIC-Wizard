/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */


#include <QtCore/QCoreApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include "qtservice.h"
#include "opennicserver.h"

class OpenNICService : public QtService<QCoreApplication>
{
	public:
		OpenNICService(int argc, char **argv)
		: QtService<QCoreApplication>(argc, argv, "OpenNIC")
		, mServer(NULL)
		{
			setServiceDescription("An OpenNIC resolver locator and sevice deamon");
			setServiceFlags(QtServiceBase::CanBeSuspended);
		}

		OpenNICServer* server() {return mServer;}

	protected:
		void start()
		{
			QCoreApplication *app = application();
			mServer = new OpenNICServer(app);
			if (!mServer->isListening())
			{
				logMessage(QString("Failed to bind to port %1").arg(mServer->serverPort()), QtServiceBase::Error);
				app->quit();
			}
			mServer->runOnce(); /* initial DNS bootstrap */
		}

		void pause()
		{
			mServer->pause();
		}

		void resume()
		{
			mServer->resume();
		}

	private:
		OpenNICServer *mServer;
};

int main(int argc, char **argv)
{
	int rc=0;
	qsrand(QDateTime::currentDateTime().toTime_t()); /* seed random numbers */
	Q_INIT_RESOURCE(opennicserver);
#if 0 // !defined(Q_WS_WIN)
	// QtService stores service settings in SystemScope, which normally require root privileges.
	// To allow testing this example as non-root, we change the directory of the SystemScope settings file.
	QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, QDir::tempPath());
	qWarning("(Example uses dummy settings file: %s/QtSoftware.conf)", QDir::tempPath().toLatin1().constData());
#endif
	OpenNICService service(argc, argv);
	return service.exec();
}


