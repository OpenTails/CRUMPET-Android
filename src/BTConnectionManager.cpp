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
#include "CommandModel.h"
#include "DeviceModel.h"
#include "gearimplementations/GearEars.h"
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
    Private()
        : tailStateCharacteristicUuid(QLatin1String("{0000ffe1-0000-1000-8000-00805f9b34fb}"))
    {
    }
    ~Private() { }

    AppSettings* appSettings{nullptr};
    QBluetoothUuid tailStateCharacteristicUuid;

    CommandModel * commandModel{nullptr};
    DeviceModel * deviceModel{nullptr};
    CommandQueue* commandQueue{nullptr};

    QBluetoothDeviceDiscoveryAgent* deviceDiscoveryAgent{nullptr};
    bool discoveryRunning{false};

    QVariantMap command;

    QBluetoothLocalDevice* localDevice{nullptr};
    int localBTDeviceState{0};
};

BTConnectionManager::BTConnectionManager(AppSettings* appSettings, QObject* parent)
    : BTConnectionManagerProxySource(parent)
    , d(new Private)
{
    d->deviceModel = new DeviceModel(this);
    setAppSettings(appSettings);

    d->commandQueue = new CommandQueue(this);

    connect(d->commandQueue, &CommandQueue::countChanged,
            this, [this](){ emit commandQueueCountChanged(d->commandQueue->count()); });

    connect(d->deviceModel, &DeviceModel::deviceMessage,
            this, [this](const QString& deviceID, const QString& deviceMessage){
                GearBase* device = d->deviceModel->getDevice(deviceID);
                if (device) {
                    Q_EMIT emit message(QString("%1 says:\n%2").arg(device->name()).arg(deviceMessage));
                }
                else {
                    Q_EMIT emit message(QString("%1 says:\n%2").arg(deviceID).arg(deviceMessage));
                }
            });
    connect(d->deviceModel, &DeviceModel::deviceBlockingMessage, this, &BTConnectionManager::blockingMessage);
    connect(d->deviceModel, &DeviceModel::countChanged,
            this, [this](){ emit deviceCountChanged(d->deviceModel->count()); });
    connect(d->deviceModel, &DeviceModel::deviceConnected, this, [this](GearBase* device){ emit deviceConnected(device->deviceID()); });
    connect(d->deviceModel, &DeviceModel::isConnectedChanged, this, &BTConnectionManager::isConnectedChanged);

    d->commandModel = new CommandModel(this);
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
    if (d->deviceModel) {
        d->deviceModel->setAppSettings(d->appSettings);
    }
    Q_EMIT bluetoothScanPermissionNameChanged(bluetoothScanPermissionName());
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
    GearBase* device;
    if (deviceID.isEmpty()) {
        device = d->deviceModel->getDevice(d->deviceModel->getDeviceID(0));
    } else {
        device = d->deviceModel->getDevice(deviceID);
    }
    if(device) {
        qDebug() << "Attempting to connect to device" << device->name();
        device->connectDevice();
    }
}

void BTConnectionManager::disconnectDevice(const QString& deviceID)
{
    if(deviceID.isEmpty()) {
        // Disconnect eeeeeverything
        for (int i = 0; i < d->deviceModel->count(); ++i) {
            const QString id{d->deviceModel->getDeviceID(i)};
            if (!id.isEmpty()) {
                disconnectDevice(id);
            }
        }
    } else {
        GearBase* device = d->deviceModel->getDevice(deviceID);
        if (device && device->isConnected()) {
            device->disconnectDevice();
            d->commandQueue->clear(device->deviceID());
            emit commandQueueChanged();
        }
    }
}

QObject* BTConnectionManager::deviceModel() const
{
    return d->deviceModel;
}

void BTConnectionManager::sendMessage(const QString &message, const QStringList& deviceIDs)
{
    d->deviceModel->sendMessage(message, deviceIDs);
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

QString BTConnectionManager::bluetoothScanPermissionName() const
{
    if (d->appSettings) {
        return d->appSettings->androidApiLevel() > 30 ? "BLUETOOTH_SCAN" : "ACCESS_FINE_LOCATION";
    }
    return {};
}

bool BTConnectionManager::isConnected() const
{
    return d->deviceModel->isConnected();
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
    const GearBase* device = d->deviceModel->getDevice(deviceID);
    if(device) {
        d->appSettings->setDeviceName(deviceID, deviceName);
        d->deviceModel->updateItem(deviceID);
    }
}

void BTConnectionManager::clearDeviceNames()
{
    appSettings()->clearDeviceNames();
    emit deviceNamesCleared();
}

void BTConnectionManager::setDeviceChecked(const QString& deviceID, bool checked)
{
    GearBase* device = d->deviceModel->getDevice(deviceID);
    if(device) {
        device->setChecked(checked);
    }
}

void BTConnectionManager::setDeviceListeningState(const QString& deviceID, int listeningMode)
{
    GearEars* ears = qobject_cast<GearEars*>(d->deviceModel->getDevice(deviceID));
    if (ears && listeningMode > -1 && listeningMode < 3) {
        ears->setListenMode(static_cast<GearEars::ListenMode>(listeningMode));
    }
}

void BTConnectionManager::setDeviceTiltState(const QString &deviceID, bool tiltState)
{
    GearEars* ears = qobject_cast<GearEars*>(d->deviceModel->getDevice(deviceID));
    if (ears) {
        ears->setTiltMode(tiltState);
    }
}

void BTConnectionManager::setDeviceCommandsFileEnabled(const QString& deviceID, const QString& filename, bool enabled)
{
    qDebug() << Q_FUNC_INFO << deviceID << filename << enabled;
    GearBase* device = d->deviceModel->getDevice(deviceID);
    if (device) {
        device->setCommandsFileEnabledState(filename, enabled);
    }
}

void BTConnectionManager::callDeviceFunction(const QString& deviceID, const QString& functionName)
{
    GearBase* device = d->deviceModel->getDevice(deviceID);
    if (device) {
        QMetaObject::invokeMethod(device, functionName.toUtf8());
    }
}
