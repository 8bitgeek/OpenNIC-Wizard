/*******************************************************************************
* Copyright (c) 2012 The OpenNIC Project                                       *
*                                                                              *
* This program is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU General Public License as published by         *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
* GNU General Public License for more details.                                 *
*                                                                              *
* You should have received a copy of the GNU General Public License            *
* along with this program.  If not, see <http://www.gnu.org/licenses/          *
********************************************************************************
* Author: Mike Sharkey <mike@pikeaero.com>                                     *
*                                                                              *
* 20120223 Initial Creation                                                    *
*******************************************************************************/
#ifndef OPENNICTESTER_H
#define OPENNICTESTER_H

#include <QThread>

class OpenNICTester : public QThread
{
    Q_OBJECT
public:
    explicit OpenNICTester(QObject *parent = 0);

signals:

public slots:

};

#endif // OPENNICTESTER_H
