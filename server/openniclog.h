/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICLOG_H
#define OPENNICLOG_H

#include <QString>

class OpenNICLog
{
	public:

		typedef enum {
			Information,
			Warning,
			Debug
		} LogLevel;

		static void					setLogFile(QString logFile) {mLogFile=logFile;}
		static QString				getLogFile() {return mLogFile;}
		static void					log(LogLevel level, QString msg);

	private:
		static QString				mLogFile;
};



#endif // OPENNICLOG_H
