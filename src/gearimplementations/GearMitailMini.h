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

#ifndef BTDEVICEMITAILMINI_H
#define BTDEVICEMITAILMINI_H

#include "GearBase.h"

class GearMitailMini : public GearBase
{
    Q_OBJECT
public:
    explicit GearMitailMini(const QBluetoothDeviceInfo& info, DeviceModel * parent = nullptr);
    ~GearMitailMini() override;

    bool isConnected() const override;
    QString version() const override;
    QString currentCall() const override;
    int batteryLevel() const override;

    void connectDevice() override;
    void disconnectDevice() override;

    QStringList defaultCommandFiles() const override;

    void sendMessage(const QString &message) override;

    Q_INVOKABLE void checkOTA() override;
    bool hasAvailableOTA() override;
    void setOtaVersion(const QString & version) override;
    QString otaVersion() override;
    Q_INVOKABLE void downloadOTAData() override;
    Q_INVOKABLE void setOTAData ( const QString& md5sum, const QByteArray& firmware ) override;
    bool hasOTAData() override;
    Q_INVOKABLE void startOTA() override;
private:
    class Private;
    Private* d;
};

#endif//BTDEVICEMITAILMINI_H
