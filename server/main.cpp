/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */

#include <QCoreApplication>
#include <QDateTime>
#include "opennicserver.h"
#include "openniclog.h"

int main(int argc, char *argv[])
{
	int rc=0;
	qsrand(QDateTime::currentDateTime().toTime_t()); /* seed random numbers */
	Q_INIT_RESOURCE(opennicserver);
	QCoreApplication a(argc, argv);
	OpenNICServer server;
	OpenNICLog::log(OpenNICLog::Information,server.copyright());
	QObject::connect(&server,SIGNAL(quit()),&a,SLOT(quit()));
	rc = a.exec();
	return rc;
}

