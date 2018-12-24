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

/**
 * Handles all connections between us and some remote bluetooth service
 */
class BTConnectionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* deviceModel READ deviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(QObject* commandModel READ commandModel NOTIFY commandModelChanged)

public:
    explicit BTConnectionManager(QObject* parent = 0);
    virtual ~BTConnectionManager();

    void connectDevice(const QBluetoothDeviceInfo& device);
    void connectClient(QLowEnergyService* remoteService);

    QObject* deviceModel() const;
    QObject* commandModel() const;

    Q_INVOKABLE void runCommand(const QString& command);

    bool isConnected() const;
public Q_SLOTS:
    void sendMessage(const QString &message);
    void connectToDevice(const QString& deviceID);
    void disconnectDevice();
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

Q_SIGNALS:
    void isConnectedChanged();
    void messageReceived(const QString &sender, const QString &message);
    void connected(const QString &name);
    void disconnected();
    void deviceModelChanged();
    void commandModelChanged();
    void message(const QString& message);

private:
    class Private;
    Private* d;
};

#endif//BTCONNECTIONMANAGER_H
