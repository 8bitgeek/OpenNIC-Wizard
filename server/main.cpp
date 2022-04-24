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

#if defined(Q_OS_UNIX)
	#include <sys/types.h>          /* See NOTES */
	#include <signal.h>
#endif

int GlobalShutdown=0;

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

#if defined(Q_OS_UNIX)

	void hupSignalHandler(int)
	{
		GlobalShutdown=1;
	}

	void termSignalHandler(int)
	{
		GlobalShutdown=1;
	}

	static int setup_unix_signal_handlers()
	{
		struct sigaction hup, term;

		hup.sa_handler = hupSignalHandler;
		sigemptyset(&hup.sa_mask);
		hup.sa_flags = 0;
		hup.sa_flags |= SA_RESTART;

		if (sigaction(SIGHUP, &hup, 0))
		return 1;

		term.sa_handler = termSignalHandler;
		sigemptyset(&term.sa_mask);
		term.sa_flags = 0;
		term.sa_flags |= SA_RESTART;

		if (sigaction(SIGTERM, &term, 0))
		return 2;

		return 0;
	}
#endif

int main(int argc, char **argv)
{
	qsrand(QDateTime::currentDateTime().toTime_t()); /* seed random numbers */
	Q_INIT_RESOURCE(opennicserver);
	
	#if 0 // !defined(Q_WS_WIN)
		// QtService stores service settings in SystemScope, which normally require root privileges.
		// To allow testing this example as non-root, we change the directory of the SystemScope settings file.
		QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, QDir::tempPath());
		qWarning("(Example uses dummy settings file: %s/QtSoftware.conf)", QDir::tempPath().toLatin1().constData());
	#endif
	
	#if defined(Q_OS_UNIX)
		setup_unix_signal_handlers();
	#endif

	OpenNICService service(argc, argv);
	return service.exec();
}


