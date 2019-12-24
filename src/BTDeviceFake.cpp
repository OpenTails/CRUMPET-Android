/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "BTDeviceFake.h"

class BTDeviceFake::Private {
public:
    Private() {}
    bool isConnected{false};
    int batteryLevel{0};
    QString currentCall;
    QString deviceID{"FA:KE:TA:IL"};
    QString version{"Fake V2"};

    QTimer batteryTimer;
};

BTDeviceFake::BTDeviceFake(const QBluetoothDeviceInfo& info, BTDeviceModel* parent)
    : BTDevice(info, parent)
    , d(new Private)
{
    d->batteryTimer.setInterval(1000);
    connect(&d->batteryTimer, &QTimer::timeout, this, [this](){
        if (d->batteryLevel > 3) {
            d->batteryLevel = 0;
        } else {
            d->batteryLevel++;
        }
        emit batteryLevelChanged(d->batteryLevel);
    });
}

BTDeviceFake::~BTDeviceFake()
{
    delete d;
}

bool BTDeviceFake::isConnected() const
{
    return d->isConnected;
}

int BTDeviceFake::batteryLevel() const
{
    return d->batteryLevel;
}

QString BTDeviceFake::currentCall() const
{
    return d->currentCall;
}

QString BTDeviceFake::deviceID() const
{
    return d->deviceID;
}

QString BTDeviceFake::version() const
{
    return d->version;
}

void BTDeviceFake::connectDevice()
{
    QTimer::singleShot(1000, this, [this](){
        d->isConnected = true;
        emit isConnectedChanged(isConnected());
        commandModel->autofill("1.0");
        d->batteryTimer.start();
    });
}

void BTDeviceFake::disconnectDevice()
{
    d->batteryTimer.stop();
    d->batteryLevel = 0;
    emit batteryLevelChanged(d->batteryLevel);
    commandModel->clear();
    d->isConnected = false;
    emit isConnectedChanged(d->isConnected);
}

void BTDeviceFake::sendMessage(const QString& message)
{
    qDebug() << "Fakery for" << message;
    CommandInfo commandInfo;
    const CommandInfoList& commands = commandModel->allCommands();
    for (const CommandInfo& command : commands) {
        if (command.command == message) {
            commandInfo = command;
            break;
        }
    }
    if(commandInfo.isValid()) {
        commandModel->setRunning(message, true);
        d->currentCall = message;
        emit currentCallChanged(message);
        QTimer::singleShot(commandInfo.duration, this, [this, message](){
            d->currentCall.clear();
            commandModel->setRunning(message, false);
            emit currentCallChanged(currentCall());
        });
    }
}
