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

#include "btconnectionmanager.h"
#include "btdevicemodel.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QLowEnergyController>

class BTConnectionManager::Private {
public:
    Private()
        : tailStateCharacteristicUuid(QLatin1String("{0000ffe1-0000-1000-8000-00805f9b34fb}"))
        , deviceModel(new BTDeviceModel)
        , discoveryAgent(nullptr)
        , btControl(nullptr)
        , tailService(nullptr)
    {}
    ~Private() {
        deviceModel->deleteLater();
    }

    QBluetoothUuid tailStateCharacteristicUuid;

    BTDeviceModel* deviceModel;
    QBluetoothServiceDiscoveryAgent *discoveryAgent;
    QLowEnergyController *btControl;
    QLowEnergyService* tailService;
    QLowEnergyCharacteristic tailCharacteristic;
    QLowEnergyDescriptor tailDescriptor;

    QString currentTailState;
};

BTConnectionManager::BTConnectionManager(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    // Create a discovery agent and connect to its signals
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            [this](const QBluetoothDeviceInfo &device){
                BTDeviceModel::Device* btDevice = new BTDeviceModel::Device();
                btDevice->name = device.name();
                btDevice->deviceID = device.address().toString();
                btDevice->deviceInfo = device;
                d->deviceModel->addDevice(btDevice);
            });

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, [](){ qDebug() << "Device discovery completed"; });
    // Start a discovery
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods());
}

BTConnectionManager::~BTConnectionManager()
{
    delete d;
}

void BTConnectionManager::connectToDevice(const QString& deviceID)
{
    const BTDeviceModel::Device* device = d->deviceModel->getDevice(deviceID);
    if(device) {
        qDebug() << "Attempting to connect to device" << device->name;
        connectDevice(device->deviceInfo);
    }
}

void BTConnectionManager::connectDevice(const QBluetoothDeviceInfo& device)
{
    if(d->btControl) {
        disconnectDevice();
    }
    d->btControl = QLowEnergyController::createCentral(device, this);
    d->btControl->setRemoteAddressType(QLowEnergyController::RandomAddress);
    if(d->tailService) {
        d->tailService->deleteLater();
        d->tailService = nullptr;
    }
    connect(d->btControl, &QLowEnergyController::serviceDiscovered,
        [](const QBluetoothUuid &gatt){
            qDebug() << "service discovered" << gatt;
        });
    connect(d->btControl, &QLowEnergyController::discoveryFinished,
            [this](){
                qDebug() << "Done!";
                QLowEnergyService *service = d->btControl->createServiceObject(QBluetoothUuid(QLatin1String("{0000ffe0-0000-1000-8000-00805f9b34fb}")));
                if (!service) {
                    qWarning() << "Cannot create QLowEnergyService for {0000ffe0-0000-1000-8000-00805f9b34fb}";
                    return;
                }
                d->tailService = service;
                connectClient(service);
            });
    connect(d->btControl, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
        this, [this](QLowEnergyController::Error error) {
            qDebug() << "Cannot connect to remote device." << error;
            disconnectDevice();
        });
    connect(d->btControl, &QLowEnergyController::connected, this, [this]() {
        qDebug() << "Controller connected. Search services...";
        d->btControl->discoverServices();
    });
    connect(d->btControl, &QLowEnergyController::disconnected, this, [this]() {
        qDebug() << "LowEnergy controller disconnected";
        disconnectDevice();
    });

    // Connect
    d->btControl->connectToDevice();
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

        foreach(const QLowEnergyCharacteristic& leChar, d->tailService->characteristics()) {
            qDebug() << "Characteristic:" << leChar.name() << leChar.uuid() << leChar.properties();
        }
        d->tailCharacteristic = d->tailService->characteristic(d->tailStateCharacteristicUuid);
        if (!d->tailCharacteristic.isValid()) {
            qDebug() << "Tail characteristic not found, this is bad";
            disconnectDevice();
            break;
        }

        d->tailDescriptor = d->tailCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        emit isConnectedChanged();

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
}

void BTConnectionManager::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "Characteristic written:" << characteristic.uuid() << newValue;
    if(d->tailStateCharacteristicUuid == characteristic.uuid()) {
        qDebug() << "This is known to us. Make the fixing thing.";
        d->currentTailState = newValue;
        emit currentTailStateChanged();
    }
}

void BTConnectionManager::disconnectDevice()
{
    if (d->btControl) {
        d->btControl->deleteLater();
        d->btControl = nullptr;
        d->tailService->deleteLater();
        d->tailService = nullptr;
        emit isConnectedChanged();
    }
}

QObject* BTConnectionManager::deviceModel() const
{
    return d->deviceModel;
}

void BTConnectionManager::sendMessage(const QString &message)
{
    if (d->tailCharacteristic.isValid()) {
        d->tailService->writeCharacteristic(d->tailCharacteristic, message.toUtf8());
    }
}

void BTConnectionManager::setCurrentTailState(const QString& newState)
{
    sendMessage(newState);
}

QString BTConnectionManager::currentTailState() const
{
    return d->currentTailState;
}

bool BTConnectionManager::isConnected() const
{
    return d->btControl;
}
