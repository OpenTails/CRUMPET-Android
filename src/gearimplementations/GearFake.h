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

#ifndef BTDEVICEFAKE_H
#define BTDEVICEFAKE_H

#include "GearBase.h"

class GearFake : public GearBase
{
    Q_OBJECT
public:
    explicit GearFake(const QBluetoothDeviceInfo& info, DeviceModel * parent = nullptr);
    ~GearFake() override;

    bool isConnected() const override;
    QString version() const override;
    QString currentCall() const override;
    int batteryLevel() const override;

    void connectDevice() override;
    void disconnectDevice() override;

    QString deviceID() const override;

    void sendMessage(const QString &message) override;
    QStringList defaultCommandFiles() const override;
private:
    class Private;
    Private* d;
};

#endif//BTDEVICEFAKE_H
