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
#ifndef OPENNICSETTINGS_H
#define OPENNICSETTINGS_H

#include <QObject>

class OpenNICSettings : public QObject
{
    Q_OBJECT
public:
    explicit OpenNICSettings(QObject *parent = 0);

signals:

public slots:

};

#endif // OPENNICSETTINGS_H
