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

#ifndef BTCONNECTIONMANAGER_H
#define BTCONNECTIONMANAGER_H

#include <QObject>

#include <QBluetoothSocket>
#include <QLowEnergyService>

#include "btdevicemodel.h"
#include "rep_btconnectionmanagerproxy_source.h"

/**
 * Handles all connections between us and some remote bluetooth service
 */
class BTConnectionManager : public BTConnectionManagerProxySource
{
    Q_OBJECT
    Q_PROPERTY(QObject* deviceModel READ deviceModel NOTIFY deviceModelChanged)
//     Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(QObject* commandModel READ commandModel NOTIFY commandModelChanged)
    Q_PROPERTY(QObject* commandQueue READ commandQueue NOTIFY commandQueueChanged)
//     Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
//     Q_PROPERTY(bool discoveryRunning READ discoveryRunning NOTIFY discoveryRunningChanged)
//     Q_PROPERTY(int deviceCount READ deviceCount NOTIFY deviceCountChanged)
//     Q_PROPERTY(int commandQueueCount READ commandQueueCount NOTIFY commandQueueCountChanged)

public:
    explicit BTConnectionManager(QObject* parent = 0);
    virtual ~BTConnectionManager();

    void connectDevice(const QBluetoothDeviceInfo& device);
    void connectClient(QLowEnergyService* remoteService);

    QObject* deviceModel() const;
    QObject* commandModel() const;
    QObject* commandQueue() const;

    Q_SLOT void runCommand(const QString& command) override;

    Q_SLOT void startDiscovery() override;
    Q_SLOT void stopDiscovery() override;
    bool discoveryRunning() const override;

    bool isConnected() const override;
    int batteryLevel() const override;
    int deviceCount() const override;
    int commandQueueCount() const override;

    void setIsConnected(bool isConnected) override { Q_UNUSED(isConnected); }
    void setBatteryLevel(int batteryLevel) override { Q_UNUSED(batteryLevel); }
    void setDiscoveryRunning(bool discoveryRunning) override { Q_UNUSED(discoveryRunning); }
    void setDeviceCount(int deviceCount) override { Q_UNUSED(deviceCount); }
    void setCommandQueueCount(int commandQueueCount) override { Q_UNUSED(commandQueueCount); }

public Q_SLOTS:
    void sendMessage(const QString &message) override;
    void connectToDevice(int deviceIndex) override;
    void disconnectDevice() override;
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

Q_SIGNALS:
//     void discoveryRunningChanged();
//     void isConnectedChanged();
//     void batteryLevelChanged();
    void messageReceived(const QString &sender, const QString &message);
    void connected(const QString &name);
    void disconnected();
    void deviceModelChanged();
    void commandModelChanged();
    void commandQueueChanged();
//     void deviceCountChanged();
//     void commandQueueCountChanged();
    void message(const QString& message);

private:
    class Private;
    Private* d;
};

#endif//BTCONNECTIONMANAGER_H
