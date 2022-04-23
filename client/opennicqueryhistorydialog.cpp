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
	QObject::connect(this,SIGNAL(finished(int)),this,SLOT(close(int)));
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
		setCursor(Qt::BusyCursor);
	}
}

/**
  * @brief is mine?
  */
bool OpenNICQueryHistoryDialog::isMine(QStringList& data)
{
	if (data.count() > 0)
	{
		QString row = data[0];
		QStringList columns = row.split(";");
		if (columns[0] == mAddress)
		{
			return true;
		}
	}
	return false;
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
			QStringList data = value.toStringList();
			if (isMine(data))
			{
				setCursor(Qt::ArrowCursor);
				history(data);
			}
		}
	}
}

/**
  * @brief apply query records <resolver>;<domain>;<nic>;<latency>;<error>;<type>;<start>;<end>;
  */
void OpenNICQueryHistoryDialog::history(QStringList& queries)
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
void OpenNICQueryHistoryDialog::timerEvent(QTimerEvent* e)
{
	if (e->timerId() == mTimer)
	{
		poll(mAddress);
	}
}

/**
  * @brief close
  */
void OpenNICQueryHistoryDialog::closeEvent(QCloseEvent* e)
{
	emit closing(this);
}

/**
  * @brief close
  */
void OpenNICQueryHistoryDialog::close(int result)
{
	emit closing(this);
}

