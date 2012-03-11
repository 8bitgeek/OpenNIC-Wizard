/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicqueryhistorydialog.h"
#include "ui_queries.h"

#include <QTableWidget>
#include <QTableWidgetItem>

OpenNICQueryHistoryDialog::OpenNICQueryHistoryDialog(OpenNICNet* net, QString address, QWidget *parent)
: QDialog(parent)
, ui(new Ui::OpenNICQueryHistory)
, mNet(net)
, mAddress(address)
{
	ui->setupUi(this);
	QObject::connect(mNet,SIGNAL(dataReady(OpenNICNet*)),this,SLOT(dataReady(OpenNICNet*)));
	setWindowIcon( QIcon( ":/images/opennic.png" ) );
	setWindowTitle( tr("OpenNIC Resolver ")+address );
	mTimer = startTimer(1000*5);
	poll(mAddress);
}

OpenNICQueryHistoryDialog::~OpenNICQueryHistoryDialog()
{
	delete ui;
}

/**
  * @brief poll for query records
  */
void OpenNICQueryHistoryDialog::poll(QString address)
{
	if (mNet->isLive() )
	{
		mNet->txPacket().set(OpenNICPacket::resolver_history,address);
		mNet->send(true);
	}
}

/**
  * @brief get here on reply
  */
void OpenNICQueryHistoryDialog::dataReady(OpenNICNet* net)
{
	QMapIterator<QString, QVariant>i(net->rxPacket().data());
	while (i.hasNext())
	{
		i.next();
		QString key = i.key();
		QVariant value = i.value();
		if ( key == OpenNICPacket::resolver_history )
		{
			history(value.toStringList());
		}
	}
}

/**
  * @brief apply query records <resolver>;<domain>;<nic>;<latency>;<error>;<type>;<start>;<end>;
  */
void OpenNICQueryHistoryDialog::history(QStringList queries)
{
	QTableWidget* table = ui->queryHistory;
	table->setRowCount(queries.count());
	for(int row=0; row < queries.count(); row++)
	{
		QStringList columns = queries[row].split(";");
		for(int col=0; col < columns.count(); col++)
		{
			table->setItem(row,col,new QTableWidgetItem(columns[col]));
		}
	}
	table->resizeColumnsToContents();
	table->resizeRowsToContents();
}

/**
  * @brief timer
  */
void OpenNICQueryHistoryDialog::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == mTimer)
	{
		poll(mAddress);
	}
}


