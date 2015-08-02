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
#include <QApplication>
#include "opennic.h"

#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QDateTime>

int main(int argc, char *argv[])
{
	int rc=0;
	qsrand(QDateTime::currentDateTime().toTime_t()); /* seed random numbers */
	Q_INIT_RESOURCE(opennic);
    QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false);
	if ( !QSystemTrayIcon::isSystemTrayAvailable() )
	{
		QMessageBox::critical(0, QObject::tr("OpenNic"), QObject::tr("No System Tray Detected."));
		rc = 1;
	}
	else
	{
        OpenNIC* on = new OpenNIC;
        QObject::connect(on,SIGNAL(quit()),&a,SLOT(quit()));
        on->connectToService();
		rc = a.exec();
        delete on;
	}
	Q_CLEANUP_RESOURCE(opennic);
	return rc;
}

