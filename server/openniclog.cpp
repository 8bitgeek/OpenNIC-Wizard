/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "openniclog.h"
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <stdio.h>

#if defined Q_OS_UNIX
QString OpenNICLog::mLogFile = "/dev/tty";
#else
QString OpenNICLog::mLogFile;
#endif

void OpenNICLog::log(LogLevel level, QString msg)
{
#if 0
	if ( !msg.trimmed().isEmpty() )
	{
		QFileInfo fi(mLogFile);
		QFile file(mLogFile);
		if ( file.open(fi.isFile() ? (QIODevice::WriteOnly|QIODevice::Append) : QIODevice::WriteOnly ) )
		{
			QString text = "["+QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss.zzz")+"]"+QString::number(level)+":"+msg.trimmed()+"\n";
			file.write(text.toAscii());
		}
		file.close();
	}
#else
    fprintf(stderr,"%s\n",msg.toAscii().data());
#endif
}

