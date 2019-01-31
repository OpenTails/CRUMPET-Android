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
#include "commandqueue.h"

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
        , deviceModel(nullptr)
        , discoveryAgent(nullptr)
        , btControl(nullptr)
        , tailService(nullptr)
        , commandModel(nullptr)
        , batteryLevel(0)
        , commandQueue(nullptr)
        , deviceDiscoveryAgent(nullptr)
        , discoveryRunning(false)
        , fakeTailMode(false)
    {
    }
    ~Private() {}

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
    CommandQueue* commandQueue;

    QBluetoothDeviceDiscoveryAgent* deviceDiscoveryAgent;
    bool discoveryRunning;

    bool fakeTailMode;
};

BTConnectionManager::BTConnectionManager(QObject* parent)
    : BTConnectionManagerProxySource(parent)
    , d(new Private)
{
    d->commandModel = new TailCommandModel(this);
    d->commandQueue = new CommandQueue(this);
    connect(d->commandQueue, &CommandQueue::countChanged, this, [this](){ emit commandQueueCountChanged(d->commandQueue->count()); });

    d->deviceModel = new BTDeviceModel(this);
    connect(d->deviceModel, &BTDeviceModel::countChanged, this, [this](){ emit deviceCountChanged(d->deviceModel->count()); });

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
        emit discoveryRunningChanged(d->discoveryRunning);
    });
    // Don't launch the discovery immediately, let's give things a change to start up...
    QTimer::singleShot(100, this, [this](){ startDiscovery(); });

    connect(&d->batteryTimer, &QTimer::timeout, [this](){ if(d->currentCall.isEmpty()) { sendMessage("BATT"); } });
    d->batteryTimer.setInterval(5 * 60000);
    d->batteryTimer.setSingleShot(false);

}

BTConnectionManager::~BTConnectionManager()
{
    delete d;
}

void BTConnectionManager::startDiscovery()
{
    d->discoveryRunning = true;
    emit discoveryRunningChanged(d->discoveryRunning);
    d->deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods());
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

void BTConnectionManager::connectToDevice(int deviceIndex)
{
    const QString& deviceID = d->deviceModel->getDeviceID(deviceIndex);
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

        d->commandModel->clear();
        emit commandModelChanged();

        d->commandQueue->clear();
        emit commandQueueChanged();

        // Get the descriptor, and turn on notifications
        d->tailDescriptor = d->tailCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        d->tailService->writeDescriptor(d->tailDescriptor, QByteArray::fromHex("0100"));

        emit isConnectedChanged(isConnected());
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
            d->commandModel->autofill(newValue);
            emit commandModelChanged();
            d->commandQueue->clear();
            emit commandQueueChanged();
            d->batteryTimer.start();
            sendMessage("BATT");
        }
        else if (d->currentCall == QLatin1String("BATT")) {
            // Return value is BAT and a number, from 0 to 4
            d->batteryLevel = newValue.right(1).toInt();
            emit batteryLevelChanged(d->batteryLevel);
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
    d->currentCall.clear();
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
        d->commandModel->clear();
        emit commandModelChanged();
        d->commandQueue->clear();
        emit commandQueueChanged();
        d->batteryLevel = 0;
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
        TailCommandModel::CommandInfo* commandInfo = d->commandModel->getCommand(message);
        if(commandInfo) {
            d->commandModel->setRunning(message, true);
            QTimer::singleShot(commandInfo->duration, this, [this, message](){ d->commandModel->setRunning(message, false); });
        }
    }
    // Don't send out another call while we're waiting to hear back... at least for a little bit
    int i = 0;
    while(!d->currentCall.isEmpty()) {
        if(++i == 100) {
            break;
        }
        qApp->processEvents();
    }
    if (d->tailCharacteristic.isValid() && d->tailService) {
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

QObject * BTConnectionManager::commandQueue() const
{
    return d->commandQueue;
}

bool BTConnectionManager::isConnected() const
{
    return d->fakeTailMode || d->btControl;
}

int BTConnectionManager::batteryLevel() const
{
    return d->batteryLevel;
}

int BTConnectionManager::deviceCount() const
{
    return d->deviceModel->count();
}

int BTConnectionManager::commandQueueCount() const
{
    return d->commandQueue->count();
}

void BTConnectionManager::setFakeTailMode(bool enableFakery)
{
    // This looks silly, but only Do The Things if we're actually trying to set it enabled, and we're not already enabled
    if(d->fakeTailMode == false && enableFakery == true) {
        d->fakeTailMode = true;
        stopDiscovery();
        QTimer::singleShot(1000, this, [this]() {
            emit isConnectedChanged(true);
            d->commandModel->autofill(QLatin1String("v1.0"));
        });
    } else {
        d->fakeTailMode = enableFakery;
    }
}
