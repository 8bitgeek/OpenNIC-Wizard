/*
 * This file is a part of OpenNIC Wizard
 * Copywrong (c) 2012-2018 Mike Sharkey
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 1776):
 * <mike@8bitgeek.net> wrote this file.
 * As long as you retain this notice you can do whatever you want with this
 * stuff. If we meet some day, and you think this stuff is worth it,
 * you can buy me a beer in return. ~ Mike Sharkey
 * ----------------------------------------------------------------------------
 */
#ifndef OPENNICQUERYHISTORYDIALOG_H
#define OPENNICQUERYHISTORYDIALOG_H

#include "opennicnet.h"

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QStringList>

namespace Ui
{
	class OpenNICQueryHistory;
}


class OpenNICQueryHistoryDialog : public QDialog
{
	Q_OBJECT
	public:
		OpenNICQueryHistoryDialog(OpenNICNet* net, QString address, QWidget *parent = 0);
		virtual ~OpenNICQueryHistoryDialog();
	signals:
		void						closing(OpenNICQueryHistoryDialog* dialog);
	protected:
		virtual bool				isMine(QStringList& data);
		virtual void				closeEvent(QCloseEvent* e);
		virtual void				timerEvent(QTimerEvent* e);
	private slots:
		void						close(int resule);
		void						dataReady(OpenNICNet* net);
		void						history(QStringList& queries);
		void						poll(QString address);
	private:
		Ui::OpenNICQueryHistory*	ui;
		OpenNICNet*					mNet;
		QString						mAddress;
		int							mTimer;

};

#endif // OPENNICQUERYHISTORYDIALOG_H
