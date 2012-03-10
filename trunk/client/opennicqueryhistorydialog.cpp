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

OpenNICQueryHistoryDialog::OpenNICQueryHistoryDialog(QWidget *parent)
: QDialog(parent)
, ui(new Ui::OpenNICQueryHistory)
{
	ui->setupUi(this);
}

OpenNICQueryHistoryDialog::~OpenNICQueryHistoryDialog()
{
	delete ui;
}
