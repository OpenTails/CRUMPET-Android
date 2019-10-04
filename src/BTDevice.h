/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef BTDEVICE_H
#define BTDEVICE_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QLowEnergyController>

#include "TailCommandModel.h"
#include "BTDeviceModel.h"

class BTDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
    Q_PROPERTY(QString currentCall READ currentCall NOTIFY currentCallChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(QString deviceID READ deviceID CONSTANT)
public:
    explicit BTDevice(const QBluetoothDeviceInfo& info, BTDeviceModel* parent = nullptr);
    ~BTDevice() override;

    QString name;
    QBluetoothDeviceInfo deviceInfo;

    QLowEnergyController *btControl{nullptr};
    QLowEnergyService* tailService{nullptr};
    QLowEnergyCharacteristic tailCharacteristic;
    QLowEnergyDescriptor tailDescriptor;

    TailCommandModel* commandModel{new TailCommandModel(this)};

    bool isConnected() const;
    Q_SIGNAL void isConnectedChanged(bool isConnected);

    QString version() const;
    Q_SIGNAL void versionChanged(QString version);

    QString currentCall() const;
    Q_SIGNAL void currentCallChanged(QString currentCall);

    int batteryLevel() const;
    Q_SIGNAL void batteryLevelChanged(int batteryLevel);

    void connectDevice();
    void disconnectDevice();

    QString deviceID() const;

    void sendMessage(const QString &message);

    Q_SIGNAL void deviceMessage(const QString& deviceID, const QString& message);
private:
    class Private;
    Private* d;
};

#endif//BTDEVICE_H
