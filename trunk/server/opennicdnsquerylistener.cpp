/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicdnsquerylistener.h"

OpenNICDnsQueryListener::OpenNICDnsQueryListener(QObject *parent)
: QObject(parent)
{
}

OpenNICDnsQueryListener::~OpenNICDnsQueryListener()
{
	QList<OpenNICDnsQuery*> queries;
	for(int n=0; n < queries.count(); n++)
	{
		OpenNICDnsQuery* query = queries[n];
		if (query->listener() == this)
		{
			delete query;
		}
	}
}

