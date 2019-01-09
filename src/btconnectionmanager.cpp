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
#include "tailcommandmodel.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QLowEnergyController>
#include <QTimer>

class BTConnectionManager::Private {
public:
    Private()
        : tailStateCharacteristicUuid(QLatin1String("{0000ffe1-0000-1000-8000-00805f9b34fb}"))
        , deviceModel(new BTDeviceModel)
        , discoveryAgent(nullptr)
        , btControl(nullptr)
        , tailService(nullptr)
        , commandModel(nullptr)
        , batteryLevel(0)
        , deviceDiscoveryAgent(nullptr)
        , discoveryRunning(false)
    {
    }
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

    TailCommandModel* commandModel;
    QString currentCall;
    int batteryLevel;
    QTimer batteryTimer;

    QBluetoothDeviceDiscoveryAgent* deviceDiscoveryAgent;
    bool discoveryRunning;
};

BTConnectionManager::BTConnectionManager(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    // Create a discovery agent and connect to its signals
    d->deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(d->deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            [this](const QBluetoothDeviceInfo &device){
                BTDeviceModel::Device* btDevice = new BTDeviceModel::Device();
                btDevice->name = device.name();
                btDevice->deviceID = device.address().toString();
                btDevice->deviceInfo = device;
                d->deviceModel->addDevice(btDevice);
            });

    connect(d->deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, [this](){
        qDebug() << "Device discovery completed";
        d->discoveryRunning = false;
        emit discoveryRunningChanged();
    });
    startDiscovery();

    connect(&d->batteryTimer, &QTimer::timeout, [this](){ if(d->currentCall.isEmpty()) { sendMessage("BATT"); } });
    d->batteryTimer.setInterval(10000);
    d->batteryTimer.setSingleShot(false);

}

BTConnectionManager::~BTConnectionManager()
{
    delete d;
}

void BTConnectionManager::startDiscovery()
{
    d->discoveryRunning = true;
    emit discoveryRunningChanged();
    d->deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods());
}

void BTConnectionManager::stopDiscovery()
{
    d->discoveryRunning = false;
    emit discoveryRunningChanged();
    d->deviceDiscoveryAgent->stop();
}

bool BTConnectionManager::discoveryRunning()
{
    return d->discoveryRunning;
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
            disconnectDevice();
        });
    connect(d->btControl, &QLowEnergyController::connected, this, [this]() {
        qDebug() << "Controller connected. Search services...";
        d->btControl->discoverServices();
    });
    connect(d->btControl, &QLowEnergyController::disconnected, this, [this]() {
        qDebug() << "LowEnergy controller disconnected";
        emit message(QLatin1String("The tail closed the connection, either by being turned off or losing power. Remember to charge your tail!"));
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

        if(d->commandModel) {
            d->commandModel->deleteLater();
        }
        d->commandModel = new TailCommandModel(this);
        emit commandModelChanged();

        // Get the descriptor, and turn on notifications
        d->tailDescriptor = d->tailCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        d->tailService->writeDescriptor(d->tailDescriptor, QByteArray::fromHex("0100"));

        emit isConnectedChanged();
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
        if (d->currentCall == QLatin1String("VER")) {
            if(!d->commandModel) {
                d->commandModel = new TailCommandModel(this);
                emit commandModelChanged();
            }
            d->commandModel->autofill(newValue);
            d->batteryTimer.start();
            sendMessage("BATT");
        }
        else if (d->currentCall == QLatin1String("BATT")) {
            // Return value is BAT and a number, from 0 to 4
            d->batteryLevel = newValue.right(1).toInt();
            emit batteryLevelChanged();
        }
        else {
            QStringList stateResult = QString(newValue).split(' ');
            if(stateResult.count() == 2) {
                if(stateResult[0] == QLatin1String("BEGIN")) {
                    d->commandModel->setRunning(stateResult[1], true);
                }
                else if(stateResult[0] == QLatin1String("END")) {
                    d->commandModel->setRunning(stateResult[1], false);
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
    d->currentCall = QLatin1String();
}

void BTConnectionManager::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "Characteristic written:" << characteristic.uuid() << newValue;
    d->currentCall = newValue;
}

void BTConnectionManager::disconnectDevice()
{
    if (d->btControl) {
        d->batteryTimer.stop();
        d->btControl->deleteLater();
        d->btControl = nullptr;
        d->tailService->deleteLater();
        d->tailService = nullptr;
        d->commandModel->deleteLater();
        d->commandModel = nullptr;
        emit commandModelChanged();
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

void BTConnectionManager::runCommand(const QString& command)
{
    sendMessage(command);
}

QObject* BTConnectionManager::commandModel() const
{
    return d->commandModel;
}

bool BTConnectionManager::isConnected() const
{
    return d->btControl;
}

int BTConnectionManager::batteryLevel() const
{
    return d->batteryLevel;
}
