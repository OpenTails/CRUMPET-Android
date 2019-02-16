/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   This file based on sample code from Kirigami
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 3, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>
 */

#ifndef IDLEMODE_H
#define IDLEMODE_H

#include <QObject>

#include "AppSettings.h"

class BTConnectionManager;

/**
 * When enabled, Idle Mode will pick a random command from the chosen categories
 * and if there are no more commands in the queue, that command will be added,
 * followed by a pause with a random duration between the minimum and maximum
 * pause durations.
 * @see AppSettings
 */
class IdleMode : public QObject
{
    Q_OBJECT
public:
    explicit IdleMode(QObject* parent = 0);
    virtual ~IdleMode();

    void setAppSettings(AppSettings* settings);
    void setConnectionManager(BTConnectionManager* connectionManager);
private:
    class Private;
    Private* d;
};

#endif//IDLEMODE_H
