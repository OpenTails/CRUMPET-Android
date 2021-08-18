/*
 *   Copyright 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "BTDeviceMitail.h"

#include <KLocalizedString>

#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include "AppSettings.h"

class BTDeviceMitail::Private {
public:
    Private(BTDeviceMitail* qq)
        : q(qq)
    {}
    ~Private() {}
    BTDeviceMitail* q;
    BTDeviceModel* parentModel;

    QString version{"(unknown)"};
    int batteryLevel{0};

    QString currentCall;
    QString currentSubCall;
    QStringList callQueue;

    QLowEnergyController* btControl{nullptr};
    QLowEnergyService* deviceService{nullptr};
    QLowEnergyCharacteristic deviceCommandWriteCharacteristic;
    QLowEnergyCharacteristic deviceCommandReadCharacteristic;

    QLowEnergyService* batteryService{nullptr};
    QLowEnergyCharacteristic batteryCharacteristic;

    QTimer pingTimer;
    QBluetoothUuid deviceServiceUuid{QLatin1String{"3af2108b-d066-42da-a7d4-55648fa0a9b6"}};
    QBluetoothUuid deviceCommandReadCharacteristicUuid{QLatin1String("{c6612b64-0087-4974-939e-68968ef294b0}")};
    QBluetoothUuid deviceCommandWriteCharacteristicUuid{QLatin1String("{5bfd6484-ddee-4723-bfe6-b653372bbfd6}")};

    int reconnectThrottle{0};
    void reconnectDevice(QObject* context)
    {
        QTimer::singleShot(0, context, [this] {
            if (btControl) {
                if (reconnectThrottle > 10) {
                    q->disconnectDevice();
                    reconnectThrottle = 0;
                    q->deviceMessage(q->deviceID(), i18nc("Error message shown when automatic reconnection has been attempted too often", "Attempted to reconnect too many times to %1 (%2). To connect to it, please check that it is on, charged, and near enough.", q->name(), q->deviceID()));
                    return;
                }
                qDebug() << q->name() << q->deviceID() << "Connection lost - attempting to reconnect.";
                q->deviceMessage(q->deviceID(), i18nc("A status message sent when the connection to a device has been lost, and we are attempting to connect again automatically", "Connection lost to %1, attempting to reconnect...", q->name()));
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

            foreach(const QLowEnergyCharacteristic& leChar, deviceService->characteristics()) {
                qDebug() << q->name() << q->deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
            }

            deviceCommandWriteCharacteristic = deviceService->characteristic(deviceCommandWriteCharacteristicUuid);
            if (!deviceCommandWriteCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "MiTail command writing characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), i18nc("A message when sent when attempting to connect to a device which does not have a specific expected feature", "It looks like this device is not a MiTail (could not find the main device writing characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            deviceCommandReadCharacteristic = deviceService->characteristic(deviceCommandReadCharacteristicUuid);
            if (!deviceCommandReadCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "MiTail command reading characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), i18nc("A message when sent when attempting to connect to a device which does not have a specific expected feature", "It looks like this device is not a MiTail (could not find the main device reading characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            q->commandModel->clear();

            // Get the descriptor, and turn on notifications
            QLowEnergyDescriptor commandUpdateDescriptor = deviceCommandReadCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (deviceCommandReadCharacteristic.properties() & QLowEnergyCharacteristic::Notify) {
                deviceService->writeDescriptor(commandUpdateDescriptor, QByteArray::fromHex("0100"));
            }
            if (deviceCommandReadCharacteristic.properties() & QLowEnergyCharacteristic::Indicate) {
                deviceService->writeDescriptor(commandUpdateDescriptor, QByteArray::fromHex("0200"));
            }

            reconnectThrottle = 0;
            emit q->isConnectedChanged(q->isConnected());
            q->sendMessage("VER"); // Ask for the version, and then react to the response...

            break;
        }
        default:
            //nothing for now
            break;
        }
    }

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
    {
        qDebug() << q->name() << q->deviceID() << "Current call is supposed to be" << currentCall << "and characteristic" << characteristic.uuid() << "NOTIFIED value change" << newValue;

        if (deviceCommandReadCharacteristicUuid == characteristic.uuid()) {
            QString theValue(newValue);
            QStringList stateResult = theValue.split(' ');
            if (theValue == QLatin1String{"System is busy now"}) {
                // Postpone what we attempted to send a few moments before trying again, as the device is currently busy
                QTimer::singleShot(1000, q, [this](){ q->sendMessage(currentSubCall); });
            }
            else if (stateResult[0] == QLatin1String{"VER"}) {
                q->reloadCommands();
                version = newValue;
                emit q->versionChanged(newValue);
                pingTimer.start();
            }
            else if (stateResult[0] == QLatin1String{"PONG"}) {
                if (currentCall != QLatin1String{"PING"}) {
                    qWarning() << q->name() << q->deviceID() << "We got an out-of-order response for a ping";
                }
            }
            else if (theValue == QLatin1String{"MiTail started"}) {
                qDebug() << q->name() << q->deviceID() << "MiTail detected the connection";
            }
            else if (stateResult.last() == QLatin1String{"BEGIN"}) {
                q->commandModel->setRunning(currentCall, true);
                // ****************************************************
                // ******************* EARLY RETURN *******************
                // ****************************************************
                return;
            }
            else if (stateResult.last() == QLatin1String{"END"}) {
                // If we've got more in the queue, send the next bit of the command
                if (callQueue.length() > 0) {
                    int pauseDuration{0};
                    QString message = callQueue.takeFirst();
                    while (message.startsWith(QLatin1String{"PAUSE"})) {
                        QStringList pauseCommand = message.split(QChar{' '});
                        int pause = pauseCommand.value(1).toInt();
                        pauseDuration += pause;
                        message = callQueue.takeFirst();
                        qDebug() << q->name() << q->deviceID() << "Found a pause, so we're now waiting" << pauseDuration << "milliseconds";
                    }
                    if (pauseDuration > 0) {
                        // Just in case some funny person stuck a pause at the end...
                        if (message.length() > 0) {
                            // Clamp the max single pause duration to 3000 ms (the conceptual human moment)
                            QTimer::singleShot(qMax(3000, pauseDuration), q, [this, message](){ q->sendMessage(message); });
                        }
                    }
                    else {
                        q->sendMessage(message);
                    }
                    // ****************************************************
                    // ******************* EARLY RETURN *******************
                    // ****************************************************
                    return;
                } else {
                    // If the queue is empty, we're done
                    q->commandModel->setRunning(currentCall, false);
                }
            }
            else {
                qDebug() << q->name() << q->deviceID() << "Unexpected response: Did not understand" << newValue;
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

BTDeviceMitail::BTDeviceMitail(const QBluetoothDeviceInfo& info, BTDeviceModel* parent)
    : BTDevice(info, parent)
    , d(new Private(this))
{
    d->parentModel = parent;
    setSupportsOTA(true);

    // The battery timer also functions as a keepalive call. If it turns
    // out to be a problem that we pull the battery this often, we can
    // add a separate ping keepalive functon.
    connect(&d->pingTimer, &QTimer::timeout,
            [this](){ if(d->currentCall.isEmpty()) { sendMessage("PING"); } });

    d->pingTimer.setTimerType(Qt::VeryCoarseTimer);
    d->pingTimer.setInterval(60000 / 2);
    d->pingTimer.setSingleShot(false);
}

BTDeviceMitail::~BTDeviceMitail()
{
    delete d;
}

void BTDeviceMitail::connectDevice()
{
    if(d->btControl) {
        disconnectDevice();
    }

    d->btControl = QLowEnergyController::createCentral(deviceInfo, this);
    d->btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);

    if(d->deviceService) {
        d->deviceService->deleteLater();
        d->deviceService = nullptr;
    }

    connect(d->btControl, &QLowEnergyController::serviceDiscovered,
        [this](const QBluetoothUuid &gatt){
            qDebug() << name() << deviceID() << "service discovered" << gatt;
        });

    connect(d->btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << name() << deviceID()<< "Done!";

                // Main control service
                d->deviceService = d->btControl->createServiceObject(d->deviceServiceUuid);
                if (!d->deviceService) {
                    qWarning() << "Cannot create QLowEnergyService for " << d->deviceServiceUuid;
                    emit deviceMessage(deviceID(), i18nc("Warning message when a fault occurred during a connection attempt", "An error occurred while connecting to your MiTail (the main service object could not be created). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }

                connect(d->deviceService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){ d->serviceStateChanged(newState); });
                connect(d->deviceService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicChanged(info, value); });
                connect(d->deviceService, &QLowEnergyService::characteristicWritten, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicWritten(info, value); });
                d->deviceService->discoverDetails();

                // Battery service
                d->batteryService = d->btControl->createServiceObject(QBluetoothUuid::BatteryService);
                if (!d->batteryService) {
                    qWarning() << "Failed to create battery service";
                    emit deviceMessage(deviceID(), i18nc("Warning message when the battery information is unavailable on a device", "An error occurred while connecting to your MiTail (the battery service was not available). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }
                else {
                    connect(d->batteryService, &QLowEnergyService::characteristicRead, this, [this](const QLowEnergyCharacteristic &, const QByteArray &value){
                        d->batteryLevel = (int)value.at(0) / 20;
                        emit batteryLevelChanged(d->batteryLevel);
                    });
                    connect(d->batteryService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic&, const QByteArray& value){
                        d->batteryLevel = (int)value.at(0) / 20;
                        emit batteryLevelChanged(d->batteryLevel);
                    });
                    connect(d->batteryService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){
                        switch (newState) {
                        case QLowEnergyService::DiscoveringServices:
                            qDebug() << name() << deviceID() << "Discovering battery services...";
                            break;
                        case QLowEnergyService::ServiceDiscovered:
                        {
                            qDebug() << name() << deviceID() << "Battery service discovered";

                            foreach(const QLowEnergyCharacteristic& leChar, d->batteryService->characteristics()) {
                                qDebug() << name() << deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
                            }

                            d->batteryCharacteristic = d->batteryService->characteristic(QBluetoothUuid::BatteryLevel);
                            if (!d->batteryCharacteristic.isValid()) {
                                qDebug() << name() << deviceID() << "MiTail battery level characteristic not found, this is bad";
                                deviceMessage(deviceID(), i18nc("Warning message when the battery information is unavailable on the device", "It looks like this device is not a MiTail (could not find the battery level characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                                disconnectDevice();
                                break;
                            }

                            // Get the descriptor, and turn on notifications
                            QLowEnergyDescriptor batteryDescriptor = d->batteryCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                            if (!batteryDescriptor.isValid()) {
                                qDebug() << "This is bad, no battery descriptor...";
                            }
                            d->batteryService->writeDescriptor(batteryDescriptor, QByteArray::fromHex("0100"));
                            d->batteryService->readCharacteristic(d->batteryCharacteristic);

                            break;
                        }
                        default:
                            //nothing for now
                            break;
                        }
                    });
                    d->batteryService->discoverDetails();
                }
            });

    connect(d->btControl, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
        this, [this](QLowEnergyController::Error error) {
            qDebug() << name() << deviceID() << "Cannot connect to remote device." << error;

            switch(error) {
                case QLowEnergyController::UnknownError:
                    emit deviceMessage(deviceID(), i18nc("Warning that some unknown error happened", "An error occurred. If you are trying to connect to your MiTail, make sure it is on and close to this device."));
                    break;
                case QLowEnergyController::RemoteHostClosedError:
                    emit deviceMessage(deviceID(), i18nc("Warning that the device disconnected itself", "The MiTail closed the connection."));
                    break;
                case QLowEnergyController::ConnectionError:
                    emit deviceMessage(deviceID(), i18nc("Warning that some connection failure occurred (usually due to low signal strength)", "Failed to connect to your MiTail. Please try again (perhaps move it closer?)"));
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
        emit deviceMessage(deviceID(), i18nc("Warning that the device itself disconnected during operation (usually due to turning off from low power)", "The MiTail closed the connection, either by being turned off or losing power. Remember to charge your tail!"));
        disconnectDevice();
    });

    // Connect
    d->btControl->connectToDevice();
}

void BTDeviceMitail::disconnectDevice()
{
    d->pingTimer.stop();
    if (d->btControl) {
        d->btControl->deleteLater();
        d->btControl = nullptr;
    }
    if (d->deviceService) {
        d->deviceService->deleteLater();
        d->deviceService = nullptr;
    }
    if (d->batteryService) {
        d->batteryService->deleteLater();
        d->batteryService = nullptr;
    }
    commandModel->clear();
    commandShorthands.clear();
//     emit commandModelChanged();
//     commandQueue->clear(); // FIXME Clear commands for this device only
//     emit commandQueueChanged();
    d->batteryLevel = 0;
    emit batteryLevelChanged(0);
    emit isConnectedChanged(isConnected());
}

bool BTDeviceMitail::isConnected() const
{
    return d->btControl;
}

QString BTDeviceMitail::version() const
{
    return d->version;
}

int BTDeviceMitail::batteryLevel() const
{
    return d->batteryLevel;
}

QString BTDeviceMitail::currentCall() const
{
    return d->currentCall;
}

QString BTDeviceMitail::deviceID() const
{
    return deviceInfo.address().toString();
}

void BTDeviceMitail::sendMessage(const QString &message)
{
    QString actualMessage{message};
    if (commandShorthands.contains(message)) {
        actualMessage = commandShorthands[message];
    }

    if (d->deviceCommandWriteCharacteristic.isValid() && d->deviceService) {
        QString actualCall{actualMessage};
        if (actualMessage.contains(';')) {
            d->callQueue = actualMessage.split(';');
            actualCall = d->callQueue.takeFirst();
        }
        if (actualMessage != message) {
            // As we're translating, we need to manually set this message as running and not trust the device to tell us
            commandModel->setRunning(message, true);
        }

        d->currentSubCall = actualCall;
        d->deviceService->writeCharacteristic(d->deviceCommandWriteCharacteristic, actualCall.toUtf8());
        d->currentCall = message;
        emit currentCallChanged(message);
    }
}

QStringList BTDeviceMitail::defaultCommandFiles() const
{
    return QStringList{QLatin1String{":/commands/digitail-builtin.crumpet"}};
}
