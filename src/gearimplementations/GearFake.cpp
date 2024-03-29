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

#include "GearFake.h"
#include "CommandPersistence.h"

#include <KLocalizedString>

#include <QFile>

class GearFake::Private {
public:
    Private() {}
    bool isConnected{false};
    int batteryLevel{-1};
    bool isCharging{false};
    QString currentCall;
    QLatin1String deviceID{"FA:KE:TA:IL"};
    QLatin1String version{"Fake V2"};

    QTimer batteryTimer;
};

GearFake::GearFake(const QBluetoothDeviceInfo& info, DeviceModel * parent)
    : GearBase(info, parent)
    , d(new Private)
{
    d->batteryTimer.setInterval(1000);
    setSupportsOTA(true);
    setHasLights(true);
    setHasShutdown(true);
    setHasNoPhoneMode(true);
    setNoPhoneModeGroups({
        i18nc("Name of the calm and relaxed group as used for no phone group selection", "Calm and Relaxed"),
        i18nc("Name of the fast and excited group as used for no phone group selection", "Fast and Excited"),
        i18nc("Name of the frustrated and tense group as used for no phone group selection", "Frustrated and Tense"),
    });
    connect(&d->batteryTimer, &QTimer::timeout, this, [this](){
        if (d->batteryLevel > 3) {
            if (d->isCharging) {
                d->isCharging = false;
                setChargingState(0);
            } else {
                d->isCharging = true;
                setChargingState(1);
            }
            d->batteryLevel = -1;
        } else {
            d->batteryLevel++;
        }
        setBatteryLevelPercent(d->batteryLevel * 25);
        Q_EMIT batteryLevelChanged(d->batteryLevel);
    });
}

GearFake::~GearFake()
{
    delete d;
}

bool GearFake::isConnected() const
{
    return d->isConnected;
}

int GearFake::batteryLevel() const
{
    return d->batteryLevel;
}

QString GearFake::currentCall() const
{
    return d->currentCall;
}

QString GearFake::deviceID() const
{
    return d->deviceID;
}

QString GearFake::version() const
{
    return d->version;
}

void GearFake::connectDevice()
{
    setIsConnecting(true);
    QTimer::singleShot(1000, this, [this](){
        d->isConnected = true;
        Q_EMIT isConnectedChanged(isConnected());
        setIsConnecting(false);
        reloadCommands();
        setKnownFirmwareMessage(i18nc("An example message to show people what the firmware message will look like for a real device", "This is a message that's supposed to inform people that there is something <b>important</b> going on with their firmware"));
        d->batteryTimer.start();
    });
}

void GearFake::disconnectDevice()
{
    d->batteryTimer.stop();
    d->batteryLevel = -1;
    Q_EMIT batteryLevelChanged(d->batteryLevel);
    commandModel->clear();
    commandShorthands.clear();
    d->isConnected = false;
    Q_EMIT isConnectedChanged(d->isConnected);
    setIsConnecting(false);
}

void GearFake::sendMessage(const QString& message)
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
        Q_EMIT currentCallChanged(message);
        QTimer::singleShot(commandInfo.duration, this, [this, message](){
            d->currentCall.clear();
            commandModel->setRunning(message, false);
            Q_EMIT currentCallChanged(currentCall());
        });
    }
}

QStringList GearFake::defaultCommandFiles() const
{
    return QStringList{QLatin1String{":/commands/mitail-builtin.crumpet"}};
}
