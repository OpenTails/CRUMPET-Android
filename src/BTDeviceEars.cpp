/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "BTDeviceEars.h"

#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include "AppSettings.h"
#include "CommandPersistence.h"

class BTDeviceEars::Private {
public:
    Private(BTDeviceEars* qq)
        : q(qq)
    {}
    ~Private() {}
    BTDeviceEars* q;
    BTDeviceModel* parentModel;

    QString version{"(unknown)"};
    int batteryLevel{0};

    QMap<QString, QString> commandShorthands;

    QString currentCall;
    QString currentSubCall;
    QStringList callQueue;

    QLowEnergyController* btControl{nullptr};
    QLowEnergyService* earsService{nullptr};
    QLowEnergyCharacteristic earsCommandWriteCharacteristic;
    QLowEnergyCharacteristic earsCommandReadCharacteristic;

    QLowEnergyService* batteryService{nullptr};
    QLowEnergyCharacteristic batteryCharacteristic;

    QTimer pingTimer;
    QBluetoothUuid earsCommandWriteCharacteristicUuid{QLatin1String("{05e026d8-b395-4416-9f8a-c00d6c3781b9}")};
    QBluetoothUuid earsCommandReadCharacteristicUuid{QLatin1String("{0b646a19-371e-4327-b169-9632d56c0e84}")};

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

            foreach(const QLowEnergyCharacteristic& leChar, earsService->characteristics()) {
                qDebug() << q->name() << q->deviceID() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
            }

            earsCommandWriteCharacteristic = earsService->characteristic(earsCommandWriteCharacteristicUuid);
            if (!earsCommandWriteCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "EarGear command writing characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), QString("It looks like this device is not an EarGear controller (could not find the main ears writing characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            earsCommandReadCharacteristic = earsService->characteristic(earsCommandReadCharacteristicUuid);
            if (!earsCommandReadCharacteristic.isValid()) {
                qDebug() << q->name() << q->deviceID() << "EarGear command reading characteristic not found, this is bad";
                q->deviceMessage(q->deviceID(), QString("It looks like this device is not an EarGear controller (could not find the main ears reading characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
                q->disconnectDevice();
                break;
            }

            q->commandModel->clear();

            // Get the descriptor, and turn on notifications
            QLowEnergyDescriptor earsDescriptor = earsCommandReadCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            earsService->writeDescriptor(earsDescriptor, QByteArray::fromHex("0100"));

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

        if (earsCommandReadCharacteristicUuid == characteristic.uuid()) {
            QString theValue(newValue);
            QStringList stateResult = theValue.split(' ');
            if (theValue == QLatin1String{"System is busy"}) {
                // Postpone what we attempted to send a few moments before trying again, as the ears are currently busy
                // ...except if we're listening, at which point don't try and do this
                // if (listeningState == ListeningFull || listeningState == ListeningOn) {
                // }
                // else {
                    QTimer::singleShot(1000, q, [this](){ q->sendMessage(currentSubCall); });
                //}
            }
            else if (stateResult[0] == QLatin1String{"VER"}) {
                CommandPersistence persistence;
                QString data;
                QFile file(QString{":/commands/eargear-base.crumpet"});
                if(file.open(QIODevice::ReadOnly)) {
                    data = file.readAll();
                }
                else {
                    qWarning() << "Failed to open the included resource containing eargear base commands, this is very much not a good thing";
                }
                file.close();
                persistence.deserialize(data);
                for (const CommandInfo &command : persistence.commands()) {
                    q->commandModel->addCommand(command);
                }
                for (const CommandShorthand& shorthand : persistence.shorthands()) {
                    commandShorthands[shorthand.command] = shorthand.expansion.join(QChar{';'});
                }

                version = newValue;
                emit q->versionChanged(newValue);
                pingTimer.start();
            }
            else if (stateResult[0] == QLatin1String{"PONG"}) {
                if (currentCall != QLatin1String{"PING"}) {
                    qWarning() << q->name() << q->deviceID() << "We got an out-of-order response for a ping";
                }
            }
            else if (theValue == QLatin1String{"EarGear started"}) {
                qDebug() << q->name() << q->deviceID() << "EarGear box detected the connection";
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
                    q->sendMessage(callQueue.takeFirst());
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

BTDeviceEars::BTDeviceEars(const QBluetoothDeviceInfo& info, BTDeviceModel* parent)
    : BTDevice(info, parent)
    , deviceInfo(info)
    , d(new Private(this))
{
    d->parentModel = parent;

    // The battery timer also functions as a keepalive call. If it turns
    // out to be a problem that we pull the battery this often, we can
    // add a separate ping keepalive functon.
    connect(&d->pingTimer, &QTimer::timeout,
            [this](){ if(d->currentCall.isEmpty()) { sendMessage("PING"); } });

    d->pingTimer.setTimerType(Qt::VeryCoarseTimer);
    d->pingTimer.setInterval(60000 / 2);
    d->pingTimer.setSingleShot(false);
}

BTDeviceEars::~BTDeviceEars()
{
    delete d;
}

void BTDeviceEars::connectDevice()
{
    if(d->btControl) {
        disconnectDevice();
    }

    d->btControl = QLowEnergyController::createCentral(deviceInfo, this);
    d->btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);

    if(d->earsService) {
        d->earsService->deleteLater();
        d->earsService = nullptr;
    }

    connect(d->btControl, &QLowEnergyController::serviceDiscovered,
        [this](const QBluetoothUuid &gatt){
            qDebug() << name() << deviceID() << "service discovered" << gatt;
        });

    connect(d->btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << name() << deviceID()<< "Done!";

                // Main control service
                d->earsService = d->btControl->createServiceObject(QBluetoothUuid(QLatin1String("{927dee04-ddd4-4582-8e42-69dc9fbfae66}")));
                if (!d->earsService) {
                    qWarning() << "Cannot create QLowEnergyService for {927dee04-ddd4-4582-8e42-69dc9fbfae66}";
                    emit deviceMessage(deviceID(), QLatin1String("An error occurred while connecting to your EarGear box (the main service object could not be created). If you feel this is in error, please try again!"));
                    disconnectDevice();
                    return;
                }

                connect(d->earsService, &QLowEnergyService::stateChanged, this, [this](QLowEnergyService::ServiceState newState){ d->serviceStateChanged(newState); });
                connect(d->earsService, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicChanged(info, value); });
                connect(d->earsService, &QLowEnergyService::characteristicWritten, this, [this](const QLowEnergyCharacteristic& info, const QByteArray& value){ d->characteristicWritten(info, value); });
                d->earsService->discoverDetails();

                // Battery service
                d->batteryService = d->btControl->createServiceObject(QBluetoothUuid::BatteryService);
                if (!d->batteryService) {
                    qWarning() << "Failed to create battery service";
                    emit deviceMessage(deviceID(), QLatin1String("An error occurred while connecting to your EarGear box (the battery service was not available). If you feel this is in error, please try again!"));
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
                                qDebug() << name() << deviceID() << "EarGear battery level characteristic not found, this is bad";
                                deviceMessage(deviceID(), QString("It looks like this device is not an EarGear controller (could not find the battery level characteristic). If you are certain that it definitely is, please report this error to The Tail Company."));
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
                    emit deviceMessage(deviceID(), QLatin1String("An error occurred. If you are trying to connect to your ears, make sure the box is on and close to this device."));
                    break;
                case QLowEnergyController::RemoteHostClosedError:
                    emit deviceMessage(deviceID(), QLatin1String("The EarGear box closed the connection."));
                    break;
                case QLowEnergyController::ConnectionError:
                    emit deviceMessage(deviceID(), QLatin1String("Failed to connect to your EarGear box. Please try again (perhaps move it closer?)"));
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
        emit deviceMessage(deviceID(), QLatin1String("The EarGear box closed the connection, either by being turned off or losing power. Remember to charge your ears!"));
        disconnectDevice();
    });

    // Connect
    d->btControl->connectToDevice();
}

void BTDeviceEars::disconnectDevice()
{
    d->pingTimer.stop();
    if (d->btControl) {
        d->btControl->deleteLater();
        d->btControl = nullptr;
    }
    if (d->earsService) {
        d->earsService->deleteLater();
        d->earsService = nullptr;
    }
    if (d->batteryService) {
        d->batteryService->deleteLater();
        d->batteryService = nullptr;
    }
    commandModel->clear();
//     emit commandModelChanged();
//     commandQueue->clear(); // FIXME Clear commands for this device only
//     emit commandQueueChanged();
    d->batteryLevel = 0;
    emit batteryLevelChanged(0);
    emit isConnectedChanged(isConnected());
}

bool BTDeviceEars::isConnected() const
{
    return d->btControl;
}

QString BTDeviceEars::version() const
{
    return d->version;
}

int BTDeviceEars::batteryLevel() const
{
    return d->batteryLevel;
}

QString BTDeviceEars::currentCall() const
{
    return d->currentCall;
}

QString BTDeviceEars::deviceID() const
{
    return deviceInfo.address().toString();
}

void BTDeviceEars::sendMessage(const QString &message)
{
    QString actualMessage{message};
    if (d->commandShorthands.contains(message)) {
        actualMessage = d->commandShorthands[message];
    }
    // Translation from tail commands to equivalent ear command streams
    // DSSP letwist ritwist letilt ritilt
    if (message == QLatin1String{"TAILHM"}) {
        actualMessage = QLatin1String{"EARHOME"};
    }
    else if(message == QLatin1String{"TAILS1"}) { // Slow Wag 1
        actualMessage = QLatin1String{"DSSP 30 30 50 50;EARHOME"};
    }
    else if(message == QLatin1String{"TAILS2"}) { // Slow Wag 2
        actualMessage = QLatin1String{"DSSP 50 50 30 30;EARHOME"};
    }
    else if(message == QLatin1String{"TAILS3"}) { // Slow Wag 3
        actualMessage = QLatin1String{"DSSP 30 30 30 30;EARHOME"};
    }
    else if(message == QLatin1String{"TAILFA"}) { // Fast Wag
        actualMessage = QLatin1String{"DSSP 50 50 50 50;EARHOME"};
    }
    else if(message == QLatin1String{"TAILSH"}) { // Short Wag
        actualMessage = QLatin1String{"EARHOME"};
    }
    else if(message == QLatin1String{"TAILHA"}) { // Happy Wag
        actualMessage = QLatin1String{"EARHOME"};
    }
    else if(message == QLatin1String{"TAILER"}) { // Stand up!
        actualMessage = QLatin1String{"DSSP 90 90 50 50;DSSP 90 90 120 120;EARHOME"};
    }
    else if(message == QLatin1String{"TAILT1"}) { // Tremble 1
        actualMessage = QLatin1String{"RITWIST 160;EARHOME"};
    }
    else if(message == QLatin1String{"TAILT2"}) { // Tremble 2
        actualMessage = QLatin1String{"RITWIST 25;EARHOME"};
    }
    else if(message == QLatin1String{"TAILET"}) { // Tremble Erect
        actualMessage = QLatin1String{"LETWIST 160;EARHOME"};
    }
    else if(message == QLatin1String{"TAILEP"}) { // High Wag
        actualMessage = QLatin1String{"LETWIST 25;EARHOME"};
    }

    if (d->earsCommandWriteCharacteristic.isValid() && d->earsService) {
        QString actualCall{actualMessage};
        if (actualMessage.contains(';')) {
            d->callQueue = actualMessage.split(';');
            actualCall = d->callQueue.takeFirst();
        }

        d->currentSubCall = actualCall;
        d->earsService->writeCharacteristic(d->earsCommandWriteCharacteristic, actualCall.toUtf8());
        d->currentCall = message;
        emit currentCallChanged(message);
    }
}
