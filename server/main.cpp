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


