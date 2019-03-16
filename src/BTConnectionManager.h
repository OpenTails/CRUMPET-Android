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
#include <QBluetoothLocalDevice>
#include <QLowEnergyService>

#include "rep_BTConnectionManagerProxy_source.h"

class AppSettings;

/**
 * Handles all connections between us and some remote bluetooth service
 */
class BTConnectionManager : public BTConnectionManagerProxySource
{
    Q_OBJECT
    Q_PROPERTY(QObject* deviceModel READ deviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(QObject* commandModel READ commandModel NOTIFY commandModelChanged)
    Q_PROPERTY(QObject* commandQueue READ commandQueue NOTIFY commandQueueChanged)

public:
    explicit BTConnectionManager(QObject* parent = nullptr);
    virtual ~BTConnectionManager();

    AppSettings* appSettings() const;
    void setAppSettings(AppSettings* appSettings);

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
    QString tailVersion() const override;
    QVariantMap command() const override;
    int bluetoothState() const override;

public Q_SLOTS:
    void sendMessage(const QString &message) override;
    void connectToDevice(int deviceIndex) override;
    void disconnectDevice() override;
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void setFakeTailMode(bool enableFakery) override;
    void setCommand(QVariantMap command) override;
    QVariantMap getCommand(const QString& command) override;
    void setLocalBTDeviceState();

Q_SIGNALS:
    void connected(const QString &name);
    void disconnected();
    void deviceModelChanged();
    void commandModelChanged();
    void commandQueueChanged();

private:
    class Private;
    Private* d;
    AppSettings *m_appSettings;

    void reconnectDevice();
};

#endif//BTCONNECTIONMANAGER_H
