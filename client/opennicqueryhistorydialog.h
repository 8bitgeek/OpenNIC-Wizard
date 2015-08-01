/*
 *   This file is a part of OpenNIC Wizard
 *   Copyright (C) 2012-2015  Mike Sharkey <mike@8bitgeek.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
