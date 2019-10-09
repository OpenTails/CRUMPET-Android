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

#include "BTDevice.h"

#include <QCoreApplication>
#include <QTimer>

#include "AppSettings.h"

class BTDevice::Private {
public:
    Private(BTDevice* qq)
        : q(qq)
    {}
    ~Private() {}
    BTDevice* q;
    BTDeviceModel* parentModel;

    QString version{"(unknown)"};
    QString currentCall;
    int batteryLevel{0};

    QTimer batteryTimer;
    QBluetoothUuid tailStateCharacteristicUuid{QLatin1String("{0000ffe1-0000-1000-8000-00805f9b34fb}")};

    int reconnectThrottle{0};
    void reconnectDevice(QObject* context)
    {
        QTimer::singleShot(0, context, [this] {
            if (q->btControl) {
                if (reconnectThrottle > 10) {
                    q->disconnectDevice();
                    q->deviceMessage(q->deviceID(), QString("Attempted to reconnect too many times to %1 (%2). To connect to it, please check that it is on, charged, and near enough.").arg(q->name).arg(q->deviceID()));
                    return;
                }
                qDebug() << "Connection lost to" << q->name << "- attempting to reconnect.";
                ++reconnectThrottle;
                q->btControl->connectToDevice();
            }
        });
    }

    void connectToDevice()
    {
        qDebug() << "Attempting to connect to device" << q->name;
        q->connectDevice();
    }

    void serviceStateChanged(QLowEnergyService::ServiceState s)
    {
        switch (s) {
        case QLowEnergyService::DiscoveringServices:
            qDebug() << "Discovering services...";
            break;
        case QLowEnergyService::ServiceDiscovered:
        {
            qDebug() << "Service discovered.";

            foreach(const QLowEnergyCharacteristic& leChar, q->tailService->characteristics()) {
                qDebug() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
            }
            q->tailCharacteristic = q->tailService->characteristic(tailStateCharacteristicUuid);
            if (!q->tailCharacteristic.isValid()) {
                qDebug() << "Tail characteristic not found, this is bad";
                q->disconnectDevice();
                break;
            }

            q->commandModel->clear();
//             emit commandModelChanged();

            // TODO only this device
//             q->commandQueue->clear();
//             emit commandQueueChanged();

            // Get the descriptor, and turn on notifications
            q->tailDescriptor = q->tailCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            q->tailService->writeDescriptor(q->tailDescriptor, QByteArray::fromHex("0100"));

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
        qDebug() << characteristic.uuid() << " NOTIFIED value change " << newValue;

        if (tailStateCharacteristicUuid == characteristic.uuid()) {
            if (currentCall == QLatin1String("VER")) {
                q->commandModel->autofill(newValue);
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
                        qDebug() << "Unexpected response: The first element of the two part message should be either BEGIN or END";
                    }
                }
                else {
                    qDebug() << "Unexpected response: The response should consist of a string of two words separated by a single space, the first word being either BEGIN or END, and the second should be the command name either just beginning its run, or having just ended its run.";
                }
            }
        }
        currentCall.clear();
        emit q->currentCallChanged(currentCall);
    }

    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        qDebug() << "Characteristic written:" << characteristic.uuid() << newValue;
        currentCall = newValue;
        emit q->currentCallChanged(currentCall);
    }
};

BTDevice::BTDevice(const QBluetoothDeviceInfo& info, BTDeviceModel* parent)
    : QObject(parent)
    , name(info.name())
    , deviceInfo(info)
    , d(new Private(this))
{
    d->parentModel = parent;
    connect(commandModel, &TailCommandModel::tailVersionChanged,
        commandModel, [this](){ /*emit tailVersionChanged(commandModel->tailVersion());*/ });

    // The battery timer also functions as a keepalive call. If it turns
    // out to be a problem that we pull the battery this often, we can
    // add a separate ping keepalive functon.
    connect(&d->batteryTimer, &QTimer::timeout,
            [this](){ if(d->currentCall.isEmpty()) { sendMessage("BATT"); } });

    d->batteryTimer.setTimerType(Qt::VeryCoarseTimer);
    d->batteryTimer.setInterval(60000 / 2);
    d->batteryTimer.setSingleShot(false);
}

BTDevice::~BTDevice()
{
    delete d;
}

void BTDevice::connectDevice()
{
    if(btControl) {
        disconnectDevice();
    }

    btControl = QLowEnergyController::createCentral(deviceInfo, this);
    btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);

    if(tailService) {
        tailService->deleteLater();
        tailService = nullptr;
    }

    connect(btControl, &QLowEnergyController::serviceDiscovered,
        [](const QBluetoothUuid &gatt){
            qDebug() << "service discovered" << gatt;
        });

    connect(btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << "Done!";
                QLowEnergyService *service = btControl->createServiceObject(QBluetoothUuid(QLatin1String("{0000ffe0-0000-1000-8000-00805f9b34fb}")));

                if (!service) {
                    qWarning() << "Cannot create QLowEnergyService for {0000ffe0-0000-1000-8000-00805f9b34fb}";
                    return;
                }

                tailService = service;
                connect(tailService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){ d->serviceStateChanged(newState); });
                connect(tailService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicChanged(info, value); });
                connect(tailService, &QLowEnergyService::characteristicWritten, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicWritten(info, value); });
                tailService->discoverDetails();
            });

    connect(btControl, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
        this, [this](QLowEnergyController::Error error) {
            qDebug() << "Cannot connect to remote device." << error;

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

    connect(btControl, &QLowEnergyController::connected, this, [this]() {
        qDebug() << "Controller connected. Search services...";
        btControl->discoverServices();
    });

    connect(btControl, &QLowEnergyController::disconnected, this, [this]() {
        qDebug() << "LowEnergy controller disconnected";
        emit deviceMessage(deviceID(), QLatin1String("The tail closed the connection, either by being turned off or losing power. Remember to charge your tail!"));
        disconnectDevice();
    });

    // Connect
    btControl->connectToDevice();
}

void BTDevice::disconnectDevice()
{
    d->batteryTimer.stop();
    btControl->deleteLater();
    btControl = nullptr;
    tailService->deleteLater();
    tailService = nullptr;
    commandModel->clear();
//     emit commandModelChanged();
//     commandQueue->clear(); // FIXME Clear commands for this device only
//     emit commandQueueChanged();
    d->batteryLevel = 0;
    emit batteryLevelChanged(0);
    emit isConnectedChanged(isConnected());
}

bool BTDevice::isConnected() const
{
    return btControl;
}

QString BTDevice::version() const
{
    return d->version;
}

int BTDevice::batteryLevel() const
{
    return d->batteryLevel;
}

QString BTDevice::currentCall() const
{
    return d->currentCall;
}

QString BTDevice::deviceID() const
{
    return deviceInfo.address().toString();
}

void BTDevice::sendMessage(const QString &message)
{
    // Don't send out another call while we're waiting to hear back... at least for a little bit
    int i = 0;
    while(!d->currentCall.isEmpty()) {
        if(++i == 100) {
            break;
        }
        qApp->processEvents();
    }
    if (tailCharacteristic.isValid() && tailService) {
        tailService->writeCharacteristic(tailCharacteristic, message.toUtf8());
    }
}
