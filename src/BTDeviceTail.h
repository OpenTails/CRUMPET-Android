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

#ifndef BTDEVICETAIL_H
#define BTDEVICETAIL_H

#include "BTDevice.h"

class BTDeviceTail : public BTDevice
{
    Q_OBJECT
public:
    explicit BTDeviceTail(const QBluetoothDeviceInfo& info, BTDeviceModel* parent = nullptr);
    ~BTDeviceTail() override;

    QBluetoothDeviceInfo deviceInfo;

    QLowEnergyController *btControl{nullptr};
    QLowEnergyService* tailService{nullptr};
    QLowEnergyCharacteristic tailCharacteristic;
    QLowEnergyDescriptor tailDescriptor;

    bool isConnected() const override;
    QString version() const override;
    QString currentCall() const override;
    int batteryLevel() const override;

    void connectDevice() override;
    void disconnectDevice() override;

    QString deviceID() const override;

    void sendMessage(const QString &message) override;
private:
    class Private;
    Private* d;
};

#endif//BTDEVICE_H
