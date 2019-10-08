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
#include "BTDeviceCommandModel.h"
#include "BTDeviceModel.h"
#include "BTDevice.h"
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
    ~Private() { }

    AppSettings* appSettings{nullptr};
    QBluetoothUuid tailStateCharacteristicUuid;

    BTDeviceCommandModel* commandModel{nullptr};
    BTDeviceModel* deviceModel{nullptr};
    BTDevice* connecedDevice{nullptr};
    CommandQueue* commandQueue{nullptr};

    QBluetoothDeviceDiscoveryAgent* deviceDiscoveryAgent = nullptr;
    bool discoveryRunning = false;

    bool fakeTailMode = false;

    QVariantMap command;

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

    connect(d->deviceModel, &BTDeviceModel::deviceMessage,
            this, [this](const QString& /*deviceID*/, const QString& deviceMessage){ emit message(deviceMessage); });
    connect(d->deviceModel, &BTDeviceModel::countChanged,
            this, [this](){ emit deviceCountChanged(d->deviceModel->count()); });
    connect(d->deviceModel, &BTDeviceModel::deviceConnected, this, [this](BTDevice* device){ emit deviceConnected(device->deviceID()); });
    connect(d->deviceModel, &BTDeviceModel::isConnectedChanged, this, &BTConnectionManager::isConnectedChanged);

    d->commandModel = new BTDeviceCommandModel(this);
    d->commandModel->setDeviceModel(d->deviceModel);

    // Create a discovery agent and connect to its signals
    d->deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(d->deviceDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), d->deviceModel, SLOT(addDevice(QBluetoothDeviceInfo)));

    connect(d->deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, [this](){
        qDebug() << "Device discovery completed";
        d->discoveryRunning = false;
        emit discoveryRunningChanged(d->discoveryRunning);
    });

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
    BTDevice* device;
    if (deviceID.isEmpty()) {
        device = d->deviceModel->getDevice(d->deviceModel->getDeviceID(0));
    } else {
        device = d->deviceModel->getDevice(deviceID);
    }
    if(device) {
        qDebug() << "Attempting to connect to device" << device->name;
        d->connecedDevice = device;
        device->connectDevice();
    }
}

void BTConnectionManager::disconnectDevice(const QString& deviceID)
{
    if (d->fakeTailMode) {
        d->fakeTailMode = false;
        emit isConnectedChanged(isConnected());
    } else {
        if(deviceID.isEmpty()) {
            // Disconnect eeeeeverything
            for (int i = 0; i < d->deviceModel->count(); ++i) {
                const QString id{d->deviceModel->getDeviceID(i)};
                if (!id.isEmpty()) {
                    disconnectDevice(id);
                }
            }
        } else {
            BTDevice* device = d->deviceModel->getDevice(deviceID);
            if (device && device->isConnected()) {
                device->disconnectDevice();
                d->commandQueue->clear(device->deviceID());
                emit commandQueueChanged();
            }
        }
    }
}

QObject* BTConnectionManager::deviceModel() const
{
    return d->deviceModel;
}

void BTConnectionManager::sendMessage(const QString &message, const QStringList& deviceIDs)
{
    if(d->fakeTailMode) {
        // Send A Message
        qDebug() << "Fakery for" << message;
        CommandInfo commandInfo = d->commandModel->getCommand(message);
        if(commandInfo.isValid()) {
            d->connecedDevice->commandModel->setRunning(message, true);
            QTimer::singleShot(commandInfo.duration, this, [this, message](){ d->connecedDevice->commandModel->setRunning(message, false); });
        }
    }
    else {
        d->deviceModel->sendMessage(message, deviceIDs);
    }
}

void BTConnectionManager::runCommand(const QString& command)
{
    sendMessage(command, QStringList{});
}

QObject* BTConnectionManager::commandModel() const
{
    return d->commandModel;
}

QObject * BTConnectionManager::commandQueue() const
{
    return d->commandQueue;
}

bool BTConnectionManager::isConnected() const
{
    return d->fakeTailMode || d->deviceModel->isConnected();
}

int BTConnectionManager::deviceCount() const
{
    return d->deviceModel->count();
}

int BTConnectionManager::commandQueueCount() const
{
    return d->commandQueue->count();
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
    if(d->commandModel) {
        const CommandInfo& actualCommand = d->commandModel->getCommand(command);
        if(actualCommand.isValid()) {
            info["category"] = actualCommand.category;
            info["command"] = actualCommand.command;
            info["duration"] = actualCommand.duration;
            info["minimumCooldown"] = actualCommand.minimumCooldown;
            info["name"] = actualCommand.name;
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
