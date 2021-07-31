/*
 *   Copyright 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef BTDEVICEMITAIL_H
#define BTDEVICEMITAIL_H

#include "BTDevice.h"

class BTDeviceMitail : public BTDevice
{
    Q_OBJECT
public:
    explicit BTDeviceMitail(const QBluetoothDeviceInfo& info, BTDeviceModel* parent = nullptr);
    ~BTDeviceMitail() override;

    bool isConnected() const override;
    QString version() const override;
    QString currentCall() const override;
    int batteryLevel() const override;

    void connectDevice() override;
    void disconnectDevice() override;

    QString deviceID() const override;
    QStringList defaultCommandFiles() const override;

    void sendMessage(const QString &message) override;
private:
    class Private;
    Private* d;
};

#endif//BTDEVICEMITAIL_H
