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
#include <QApplication>
#include "opennic.h"

#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QDateTime>
#include <QRandomGenerator>

int main(int argc, char *argv[])
{
	int rc=0;
	QRandomGenerator(QDateTime::currentDateTime().toTime_t()); /* seed random numbers */
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

