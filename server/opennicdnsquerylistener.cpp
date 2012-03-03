/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicdnsquerylistener.h"
#include <QMultiHash>

OpenNICDnsQueryListener::OpenNICDnsQueryListener(QObject *parent)
: QObject(parent)
{
}

OpenNICDnsQueryListener::~OpenNICDnsQueryListener()
{
	const QMultiHash<OpenNICDnsQueryListener*,OpenNICDnsQuery*>& queries = OpenNICDnsQuery::queries();
	QMultiHash<OpenNICDnsQueryListener*,OpenNICDnsQuery*>::const_iterator i = queries.find(this);
	while (i != queries.end() && i.key() == this)
	{
		OpenNICDnsQuery* query = i.value();
		++i;
		query->deleteLater();
	}
}

