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

#include "BTDeviceTail.h"

#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include "AppSettings.h"
#include "CommandPersistence.h"

class BTDeviceTail::Private {
public:
    Private(BTDeviceTail* qq)
        : q(qq)
    {}
    ~Private() {}
    BTDeviceTail* q;
    BTDeviceModel* parentModel;

    QString version{"(unknown)"};
    QString currentCall;
    int batteryLevel{0};

    QLowEnergyController* btControl{nullptr};
    QLowEnergyService* tailService{nullptr};
    QLowEnergyCharacteristic tailCharacteristic;
    QLowEnergyDescriptor tailDescriptor;

    QTimer batteryTimer;
    QBluetoothUuid tailStateCharacteristicUuid{QLatin1String("{0000ffe1-0000-1000-8000-00805f9b34fb}")};

    int reconnectThrottle{0};
    void reconnectDevice(QObject* context)
    {
        QTimer::singleShot(0, context, [this] {
            if (btControl) {
                if (reconnectThrottle > 10) {
                    q->disconnectDevice();
                    reconnectThrottle = 0;
                    q->deviceMessage(q->deviceID(), QString("Attempted to reconnect too many times to %1 (%2). To connect to it, please check that it is on, charged, and near enough.").arg(q->name()).arg(q->deviceID()));
                    return;
                }
                qDebug() << q->name() << q->deviceID() << "Connection lost - attempting to reconnect.";
                q->deviceMessage(q->deviceID(), QString("Connection lost to %1, attempting to reconnect...").arg(q->name()));
                ++reconnectThrottle;
                btControl->connectToDevice();
            }
        });
    }

    void connectToDevice()
    {
        qDebug() << q->name() << q->deviceID() << "Attempting to connect to device";
        q->connectDevice();
    }

    void serviceStateChanged(QLowEnergyService::ServiceState s)
    {
        switch (s) {
        case QLowEnergyService::DiscoveringServices:
            qDebug() << q->name() << q->deviceID() << "Discovering services...";
            break;
        case QLowEnergyService::ServiceDiscovered:
        {
            qDebug() << q->name() << q->deviceID() << "Service discovered.";

            foreach(const QLowEnergyCharacteristic& leChar, tailService->characteristics()) {
                qDebug() << q->name() << q->deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
            }
            tailCharacteristic = tailService->characteristic(tailStateCharacteristicUuid);
            if (!tailCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "Tail characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), QString("It looks like this device is not a DIGITAiL (could not find the tail characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            q->commandModel->clear();

            // Get the descriptor, and turn on notifications
            tailDescriptor = tailCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            tailService->writeDescriptor(tailDescriptor, QByteArray::fromHex("0100"));

            reconnectThrottle = 0;
            emit q->isConnectedChanged(q->isConnected());
            q->sendMessage("VER"); // Ask for the tail version, and then react to the response...

            break;
        }
        default:
            //nothing for now
            break;
        }
    }

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        qDebug() << q->name() << q->deviceID() << "Current call is" << currentCall << "and characteristic" << characteristic.uuid() << "NOTIFIED value change" << newValue;

        if (tailStateCharacteristicUuid == characteristic.uuid()) {
            if (currentCall == QLatin1String("VER")) {
                CommandPersistence persistence;
                QString data;
                QFile file(QString{":/commands/digitail-builtin.crumpet"});
                if(file.open(QIODevice::ReadOnly)) {
                    data = file.readAll();
                }
                else {
                    qWarning() << "Failed to open the included resource containing the digitail builtin commands, this is very much not a good thing";
                }
                file.close();
                persistence.deserialize(data);
                for (const CommandInfo &command : persistence.commands()) {
                    q->commandModel->addCommand(command);
                }
                // This'll want adding in... but let's leave it for now
//                 for (const CommandShorthand& shorthand : persistence.shorthands()) {
//                     commandShorthands[shorthand.command] = shorthand.expansion.join(QChar{';'});
//                 }

                version = newValue;
                emit q->versionChanged(newValue);
                batteryTimer.start();
                q->sendMessage("BATT");
            }
            else {
                QString theValue(newValue);
                QStringList stateResult = theValue.split(' ');
                // Return value for BATT calls is BAT and a number, from 0 to 4,
                // unfortunately without a space, so we have to specialcase it a bit
                if(theValue.startsWith(QLatin1String("BAT"))) {
                    batteryLevel = theValue.right(1).toInt();
                    emit q->batteryLevelChanged(batteryLevel);
                }
                else if(stateResult.count() == 2) {
                    if(stateResult[0] == QLatin1String("BEGIN")) {
                        q->commandModel->setRunning(stateResult[1], true);
                    }
                    else if(stateResult[0] == QLatin1String("END")) {
                        q->commandModel->setRunning(stateResult[1], false);
                    }
                    else {
                        qDebug() << q->name() << q->deviceID() << "Unexpected response: The first element of the two part message should be either BEGIN or END";
                    }
                }
                else {
                    qDebug() << q->name() << q->deviceID() << "Unexpected response: The response should consist of a string of two words separated by a single space, the first word being either BEGIN or END, and the second should be the command name either just beginning its run, or having just ended its run.";
                }
            }
        }
        currentCall.clear();
        emit q->currentCallChanged(currentCall);
    }

    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        qDebug() << q->name() << q->deviceID() << "Characteristic written:" << characteristic.uuid() << newValue;
        currentCall = newValue;
        emit q->currentCallChanged(currentCall);
    }
};

BTDeviceTail::BTDeviceTail(const QBluetoothDeviceInfo& info, BTDeviceModel* parent)
    : BTDevice(info, parent)
    , deviceInfo(info)
    , d(new Private(this))
{
    d->parentModel = parent;

    // The battery timer also functions as a keepalive call. If it turns
    // out to be a problem that we pull the battery this often, we can
    // add a separate ping keepalive functon.
    connect(&d->batteryTimer, &QTimer::timeout,
            [this](){ if(d->currentCall.isEmpty()) { sendMessage("BATT"); } });

    d->batteryTimer.setTimerType(Qt::VeryCoarseTimer);
    d->batteryTimer.setInterval(60000 / 2);
    d->batteryTimer.setSingleShot(false);
}

BTDeviceTail::~BTDeviceTail()
{
    delete d;
}

void BTDeviceTail::connectDevice()
{
    if(d->btControl) {
        disconnectDevice();
    }

    d->btControl = QLowEnergyController::createCentral(deviceInfo, this);
    d->btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);

    if(d->tailService) {
        d->tailService->deleteLater();
        d->tailService = nullptr;
    }

    connect(d->btControl, &QLowEnergyController::serviceDiscovered,
        [this](const QBluetoothUuid &gatt){
            qDebug() << name() << deviceID() << "service discovered" << gatt;
        });

    connect(d->btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << name() << deviceID()<< "Done!";
                QLowEnergyService *service = d->btControl->createServiceObject(QBluetoothUuid(QLatin1String("{0000ffe0-0000-1000-8000-00805f9b34fb}")));

                if (!service) {
                    qWarning() << "Cannot create QLowEnergyService for {0000ffe0-0000-1000-8000-00805f9b34fb}";
                    emit deviceMessage(deviceID(), QLatin1String("An error occured while connecting to your tail (the service object could not be created). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }

                d->tailService = service;
                connect(d->tailService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){ d->serviceStateChanged(newState); });
                connect(d->tailService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicChanged(info, value); });
                connect(d->tailService, &QLowEnergyService::characteristicWritten, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicWritten(info, value); });
                d->tailService->discoverDetails();
            });

    connect(d->btControl, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
        this, [this](QLowEnergyController::Error error) {
            qDebug() << name() << deviceID() << "Cannot connect to remote device." << error;

            switch(error) {
                case QLowEnergyController::UnknownError:
                    emit deviceMessage(deviceID(), QLatin1String("An error occurred. If you are trying to connect to your tail, make sure it is on and close to this device."));
                    break;
                case QLowEnergyController::RemoteHostClosedError:
                    emit deviceMessage(deviceID(), QLatin1String("The tail closed the connection."));
                    break;
                case QLowEnergyController::ConnectionError:
                    emit deviceMessage(deviceID(), QLatin1String("Failed to connect to your tail. Please try again (perhaps move it closer?)"));
                    break;
                default:
                    break;
            }

            if (d->parentModel->appSettings()->autoReconnect()) {
                d->reconnectDevice(this);
            } else {
                disconnectDevice();
            }
        });

    connect(d->btControl, &QLowEnergyController::connected, this, [this]() {
        qDebug() << name() << deviceID() << "Controller connected. Search services...";
        d->btControl->discoverServices();
    });

    connect(d->btControl, &QLowEnergyController::disconnected, this, [this]() {
        qDebug() << name() << deviceID() << "LowEnergy controller disconnected";
        emit deviceMessage(deviceID(), QLatin1String("The tail closed the connection, either by being turned off or losing power. Remember to charge your tail!"));
        disconnectDevice();
    });

    // Connect
    d->btControl->connectToDevice();
}

void BTDeviceTail::disconnectDevice()
{
    d->batteryTimer.stop();
    d->btControl->deleteLater();
    d->btControl = nullptr;
    d->tailService->deleteLater();
    d->tailService = nullptr;
    commandModel->clear();
//     emit commandModelChanged();
//     commandQueue->clear(); // FIXME Clear commands for this device only
//     emit commandQueueChanged();
    d->batteryLevel = 0;
    emit batteryLevelChanged(0);
    emit isConnectedChanged(isConnected());
}

bool BTDeviceTail::isConnected() const
{
    return d->btControl;
}

QString BTDeviceTail::version() const
{
    return d->version;
}

int BTDeviceTail::batteryLevel() const
{
    return d->batteryLevel;
}

QString BTDeviceTail::currentCall() const
{
    return d->currentCall;
}

QString BTDeviceTail::deviceID() const
{
    return deviceInfo.address().toString();
}

void BTDeviceTail::sendMessage(const QString &message)
{
    // Don't send out another call while we're waiting to hear back... at least for a little bit
    int i = 0;
    while(!d->currentCall.isEmpty()) {
        if(++i == 1000) {
            break;
        }
        qApp->processEvents();
    }
    if (d->tailCharacteristic.isValid() && d->tailService) {
        d->tailService->writeCharacteristic(d->tailCharacteristic, message.toUtf8());
        d->currentCall = message;
        emit currentCallChanged(message);
    }
}
