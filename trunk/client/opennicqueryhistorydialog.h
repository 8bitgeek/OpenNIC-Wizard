/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#ifndef OPENNICQUERYHISTORYDIALOG_H
#define OPENNICQUERYHISTORYDIALOG_H

#include <QDialog>


namespace Ui
{
	class OpenNICQueryHistory;
}


class OpenNICQueryHistoryDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit OpenNICQueryHistoryDialog(QWidget *parent = 0);

	private:
		Ui::OpenNICQueryHistory*	ui;

};

#endif // OPENNICQUERYHISTORYDIALOG_H
