/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "BTConnectionManager.h"
#include "BTDeviceModel.h"
#include "TailCommandModel.h"
#include "CommandQueue.h"
#include "AppSettings.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QCoreApplication>
#include <QLowEnergyController>
#include <QTimer>

class BTConnectionManager::Private {
public:
    Private(AppSettings* appSettings)
        : appSettings(appSettings),
          tailStateCharacteristicUuid(QLatin1String("{0000ffe1-0000-1000-8000-00805f9b34fb}"))
    {
    }
    ~Private() {}

    AppSettings* appSettings{nullptr};
    QBluetoothUuid tailStateCharacteristicUuid;

    BTDeviceModel* deviceModel{nullptr};
    BTDevice* connecedDevice{nullptr};
    CommandQueue* commandQueue{nullptr};

    QBluetoothDeviceDiscoveryAgent* deviceDiscoveryAgent = nullptr;
    bool discoveryRunning = false;

    bool fakeTailMode = false;

    QVariantMap command;
    QTimer batteryTimer;

    void reconnectDevice(QObject* context)
    {
        QTimer::singleShot(0, context, [this] {
            if (connecedDevice && connecedDevice->btControl) {
                connecedDevice->btControl->connectToDevice();
            }
        });
    }

    QBluetoothLocalDevice* localDevice = nullptr;
    int localBTDeviceState = 0;
};

BTConnectionManager::BTConnectionManager(AppSettings* appSettings, QObject* parent)
    : BTConnectionManagerProxySource(parent)
    , d(new Private(appSettings))
{

    d->commandQueue = new CommandQueue(this);

    connect(d->commandQueue, &CommandQueue::countChanged,
            this, [this](){ emit commandQueueCountChanged(d->commandQueue->count()); });

    d->deviceModel = new BTDeviceModel(this);
    d->deviceModel->setAppSettings(d->appSettings);

    connect(d->deviceModel, &BTDeviceModel::countChanged,
            this, [this](){ emit deviceCountChanged(d->deviceModel->count()); });

    // Create a discovery agent and connect to its signals
    d->deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(d->deviceDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), d->deviceModel, SLOT(addDevice(QBluetoothDeviceInfo)));

    connect(d->deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, [this](){
        qDebug() << "Device discovery completed";
        d->discoveryRunning = false;
        emit discoveryRunningChanged(d->discoveryRunning);
    });

    // The battery timer also functions as a keepalive call. If it turns
    // out to be a problem that we pull the battery this often, we can
    // add a separate ping keepalive functon.
    //FIXME iterate over all connected devices (don't cache, just use model...)
    connect(&d->batteryTimer, &QTimer::timeout,
            [this](){ if(d->connecedDevice && d->connecedDevice->currentCall.isEmpty()) { sendMessage("BATT"); } });

    d->batteryTimer.setInterval(60000 / 2);
    d->batteryTimer.setSingleShot(false);

    d->localDevice = new QBluetoothLocalDevice(this);
    connect(d->localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)), this, SLOT(setLocalBTDeviceState()));
    setLocalBTDeviceState();
}

BTConnectionManager::~BTConnectionManager()
{
    delete d;
}

AppSettings* BTConnectionManager::appSettings() const
{
    return d->appSettings;
}

void BTConnectionManager::setAppSettings(AppSettings* appSettings)
{
    d->appSettings = appSettings;
    d->deviceModel->setAppSettings(d->appSettings);
}

void BTConnectionManager::setLocalBTDeviceState()
{   //0-off, 1-on, 2-no device
    //TODO: use enum?
    int newState = 1;
    if (!d->localDevice->isValid()) {
        newState = 2;
    } else if (d->localDevice->hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        newState = 0;
    }

    bool changed = (newState != d->localBTDeviceState);
    d->localBTDeviceState = newState;
    if (changed) {
        emit bluetoothStateChanged(newState);
    }
}

void BTConnectionManager::startDiscovery()
{
    if (!d->discoveryRunning) {
        d->discoveryRunning = true;
        emit discoveryRunningChanged(d->discoveryRunning);
        d->deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods());
    }
}

void BTConnectionManager::stopDiscovery()
{
    d->discoveryRunning = false;
    emit discoveryRunningChanged(d->discoveryRunning);
    d->deviceDiscoveryAgent->stop();
}

bool BTConnectionManager::discoveryRunning() const
{
    return d->discoveryRunning;
}

void BTConnectionManager::connectToDevice(const QString& deviceID)
{
    const BTDevice* device;
    if (deviceID.isEmpty()) {
        device = d->deviceModel->getDevice(d->deviceModel->getDeviceID(0));
    } else {
        device = d->deviceModel->getDevice(deviceID);
    }
    if(device) {
        qDebug() << "Attempting to connect to device" << device->name;
        connectDevice(device->deviceInfo);
    }
}

void BTConnectionManager::connectDevice(const QBluetoothDeviceInfo& device)
{
    if(d->connecedDevice->btControl) {
        disconnectDevice();
    }

    d->connecedDevice->btControl = QLowEnergyController::createCentral(device, this);
    d->connecedDevice->btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);
    connect(d->connecedDevice->btControl, &QObject::destroyed, this, [this](){ emit currentDeviceIDChanged(QLatin1String{}); } );

    if(d->connecedDevice->tailService) {
        d->connecedDevice->tailService->deleteLater();
        d->connecedDevice->tailService = nullptr;
    }

    connect(d->connecedDevice->btControl, &QLowEnergyController::serviceDiscovered,
        [](const QBluetoothUuid &gatt){
            qDebug() << "service discovered" << gatt;
        });

    connect(d->connecedDevice->btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << "Done!";
                QLowEnergyService *service = d->connecedDevice->btControl->createServiceObject(QBluetoothUuid(QLatin1String("{0000ffe0-0000-1000-8000-00805f9b34fb}")));

                if (!service) {
                    qWarning() << "Cannot create QLowEnergyService for {0000ffe0-0000-1000-8000-00805f9b34fb}";
                    return;
                }

                d->connecedDevice->tailService = service;
                connectClient(service);
            });

    connect(d->connecedDevice->btControl, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
        this, [this](QLowEnergyController::Error error) {
            qDebug() << "Cannot connect to remote device." << error;

            switch(error) {
                case QLowEnergyController::UnknownError:
                    emit message(QLatin1String("An error occurred. If you are trying to connect to your tail, make sure it is on and close to this device."));
                    break;
                case QLowEnergyController::RemoteHostClosedError:
                    emit message(QLatin1String("The tail closed the connection."));
                    break;
                case QLowEnergyController::ConnectionError:
                    emit message(QLatin1String("Failed to connect to your tail. Please try again (perhaps move it closer?)"));
                    break;
                default:
                    break;
            }

            if (d->fakeTailMode) {
                qDebug() << "We're doing the tail fakery, ignore the connection failure stuff";
            } else {
                if (d->appSettings->autoReconnect()) {
                    d->reconnectDevice(this);
                } else {
                    disconnectDevice();
                }
            }
        });

    connect(d->connecedDevice->btControl, &QLowEnergyController::connected, this, [this]() {
        qDebug() << "Controller connected. Search services...";
        d->connecedDevice->btControl->discoverServices();
    });

    connect(d->connecedDevice->btControl, &QLowEnergyController::disconnected, this, [this]() {
        qDebug() << "LowEnergy controller disconnected";
        emit message(QLatin1String("The tail closed the connection, either by being turned off or losing power. Remember to charge your tail!"));
        disconnectDevice();
    });

    // Connect
    d->connecedDevice->btControl->connectToDevice();
}

void BTConnectionManager::reconnectDevice()
{
    QTimer::singleShot(0, this, [this] {
        if (d->connecedDevice->btControl) {
            d->connecedDevice->btControl->connectToDevice();
        }
    });
}

void BTConnectionManager::connectClient(QLowEnergyService* remoteService)
{
    connect(remoteService, &QLowEnergyService::stateChanged, this, &BTConnectionManager::serviceStateChanged);
    connect(remoteService, &QLowEnergyService::characteristicChanged, this, &BTConnectionManager::characteristicChanged);
    connect(remoteService, &QLowEnergyService::characteristicWritten, this, &BTConnectionManager::characteristicWritten);
    remoteService->discoverDetails();
}

void BTConnectionManager::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::DiscoveringServices:
        qDebug() << "Discovering services...";
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        qDebug() << "Service discovered.";

        foreach(const QLowEnergyCharacteristic& leChar, d->connecedDevice->tailService->characteristics()) {
            qDebug() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
        }
        d->connecedDevice->tailCharacteristic = d->connecedDevice->tailService->characteristic(d->tailStateCharacteristicUuid);
        if (!d->connecedDevice->tailCharacteristic.isValid()) {
            qDebug() << "Tail characteristic not found, this is bad";
            disconnectDevice();
            break;
        }

        d->connecedDevice->commandModel->clear();
        emit commandModelChanged();

        // TODO only this device
        d->commandQueue->clear();
        emit commandQueueChanged();

        // Get the descriptor, and turn on notifications
        d->connecedDevice->tailDescriptor = d->connecedDevice->tailCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        d->connecedDevice->tailService->writeDescriptor(d->connecedDevice->tailDescriptor, QByteArray::fromHex("0100"));

        emit isConnectedChanged(isConnected());
        emit currentDeviceIDChanged(currentDeviceID());
        sendMessage("VER"); // Ask for the tail version, and then react to the response...

        break;
    }
    default:
        //nothing for now
        break;
    }
}

void BTConnectionManager::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << characteristic.uuid() << " NOTIFIED value change " << newValue;

    if (d->tailStateCharacteristicUuid == characteristic.uuid()) {
        if (d->connecedDevice->currentCall == QLatin1String("VER")) {
            d->connecedDevice->commandModel->autofill(newValue);
            emit commandModelChanged();
            d->commandQueue->clear();
            emit commandQueueChanged();
            d->batteryTimer.start();
            sendMessage("BATT");
        }
        else {
            QString theValue(newValue);
            QStringList stateResult = theValue.split(' ');
            // Return value for BATT calls is BAT and a number, from 0 to 4,
            // unfortunately without a space, so we have to specialcase it a bit
            if(theValue.startsWith(QLatin1String("BAT"))) {
                d->connecedDevice->batteryLevel = theValue.right(1).toInt();
                emit batteryLevelChanged(d->connecedDevice->batteryLevel);
            }
            else if(stateResult.count() == 2) {
                if(stateResult[0] == QLatin1String("BEGIN")) {
                    d->connecedDevice->commandModel->setRunning(stateResult[1], true);
                }
                else if(stateResult[0] == QLatin1String("END")) {
                    d->connecedDevice->commandModel->setRunning(stateResult[1], false);
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
    d->connecedDevice->currentCall.clear();
}

void BTConnectionManager::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "Characteristic written:" << characteristic.uuid() << newValue;
    d->connecedDevice->currentCall = newValue;
}

void BTConnectionManager::disconnectDevice()
{
    if (d->fakeTailMode) {
        d->fakeTailMode = false;
        emit isConnectedChanged(isConnected());
    } else if (d->connecedDevice->btControl) {
        d->batteryTimer.stop(); // FIXME Don't until all connected devices are disconnected
        d->connecedDevice->btControl->deleteLater();
        d->connecedDevice->btControl = nullptr;
        d->connecedDevice->tailService->deleteLater();
        d->connecedDevice->tailService = nullptr;
        d->connecedDevice->commandModel->clear();
        emit commandModelChanged();
        d->connecedDevice = nullptr;
        d->commandQueue->clear(); // FIXME Clear commands for this device only
        emit commandQueueChanged();
        d->connecedDevice->batteryLevel = 0;
        emit batteryLevelChanged(0);
        emit isConnectedChanged(isConnected());
    }
}

QObject* BTConnectionManager::deviceModel() const
{
    return d->deviceModel;
}

void BTConnectionManager::sendMessage(const QString &message)
{
    if(d->fakeTailMode) {
        // Send A Message
        qDebug() << "Fakery for" << message;
        TailCommandModel::CommandInfo* commandInfo = d->connecedDevice->commandModel->getCommand(message);
        if(commandInfo) {
            d->connecedDevice->commandModel->setRunning(message, true);
            QTimer::singleShot(commandInfo->duration, this, [this, message](){ d->connecedDevice->commandModel->setRunning(message, false); });
        }
    }
    // Don't send out another call while we're waiting to hear back... at least for a little bit
    int i = 0;
    while(!d->connecedDevice->currentCall.isEmpty()) {
        if(++i == 100) {
            break;
        }
        qApp->processEvents();
    }
    if (d->connecedDevice->tailCharacteristic.isValid() && d->connecedDevice->tailService) {
        d->connecedDevice->tailService->writeCharacteristic(d->connecedDevice->tailCharacteristic, message.toUtf8());
    }
}

void BTConnectionManager::runCommand(const QString& command)
{
    sendMessage(command);
}

QObject* BTConnectionManager::commandModel() const
{
    return d->connecedDevice->commandModel;
}

QObject * BTConnectionManager::commandQueue() const
{
    return d->commandQueue;
}

bool BTConnectionManager::isConnected() const
{
    return d->fakeTailMode || (d->connecedDevice && d->connecedDevice->btControl);
}

int BTConnectionManager::batteryLevel() const
{
    return d->connecedDevice->batteryLevel;
}

int BTConnectionManager::deviceCount() const
{
    return d->deviceModel->count();
}

int BTConnectionManager::commandQueueCount() const
{
    return d->commandQueue->count();
}

QString BTConnectionManager::tailVersion() const
{
    return d->connecedDevice->commandModel->tailVersion();
}

QString BTConnectionManager::currentDeviceID() const
{
    // We should check for d->btControl because we may have fakeTailMode
    if(isConnected() && d->connecedDevice->btControl) {
        return d->connecedDevice->btControl->remoteAddress().toString();
    }
    return QString();
}

int BTConnectionManager::bluetoothState() const
{
    return d->localBTDeviceState;
}

void BTConnectionManager::setFakeTailMode(bool enableFakery)
{
    // This looks silly, but only Do The Things if we're actually trying to set it enabled, and we're not already enabled
    if(d->fakeTailMode == false && enableFakery == true) {
        d->fakeTailMode = true;
        stopDiscovery();
        QTimer::singleShot(1000, this, [this]() {
            emit isConnectedChanged(true);
            d->connecedDevice->commandModel->autofill(QLatin1String("v1.0"));
        });
    } else {
        d->fakeTailMode = enableFakery;
    }
}

void BTConnectionManager::setCommand(QVariantMap command)
{
    QString actualCommand = command["command"].toString();
    if(actualCommand.startsWith("pause")) {
        d->command["category"] = "";
        d->command["command"] = actualCommand;
        d->command["duration"] = actualCommand.split(':').last().toInt() * 1000;
        d->command["minimumCooldown"] = 0;
        d->command["name"] = "Pause";
    } else {
        d->command = getCommand(command["command"].toString());
    }
    emit commandChanged(d->command);
}

QVariantMap BTConnectionManager::command() const
{
    return d->command;
}

QVariantMap BTConnectionManager::getCommand(const QString& command)
{
    QVariantMap info;
    if(d->connecedDevice && d->connecedDevice->commandModel) {
        TailCommandModel::CommandInfo* actualCommand = d->connecedDevice->commandModel->getCommand(command);
        if(actualCommand) {
            info["category"] = actualCommand->category;
            info["command"] = actualCommand->command;
            info["duration"] = actualCommand->duration;
            info["minimumCooldown"] = actualCommand->minimumCooldown;
            info["name"] = actualCommand->name;
        }
    }
    return info;
}

void BTConnectionManager::setDeviceName(const QString& deviceID, const QString& deviceName)
{
    const BTDevice* device = d->deviceModel->getDevice(deviceID);
    if(device) {
        d->appSettings->setDeviceName(device->deviceInfo.address().toString(), deviceName);
        d->deviceModel->updateItem(deviceID);
    }
}

void BTConnectionManager::clearDeviceNames()
{
    appSettings()->clearDeviceNames();
    emit deviceNamesCleared();
}
