/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef BTDEVICEEARS_H
#define BTDEVICEEARS_H

#include "GearBase.h"

class GearEars : public GearBase
{
    Q_OBJECT
    Q_PROPERTY(ListenMode listenMode READ listenMode WRITE setListenMode NOTIFY listenModeChanged)
    Q_PROPERTY(bool micsSwapped READ micsSwapped NOTIFY micsSwappedChanged)
    Q_PROPERTY(bool hasTilt READ hasTilt NOTIFY hasTiltChanged)
    Q_PROPERTY(bool canBalanceListening READ canBalanceListening NOTIFY canBalanceListeningChanged)
    Q_PROPERTY(bool tiltEnabled READ tiltEnabled NOTIFY tiltEnabledChanged)
public:
    explicit GearEars(const QBluetoothDeviceInfo& info, DeviceModel * parent = nullptr);
    ~GearEars() override;

    enum ListenMode {
        ListenModeOff, ///< Microphones off
        ListenModeOn, ///< The iOS-friendly on-board listen mode
        ListenModeFull ///< Full listening support, including feedback for all values
    };
    Q_ENUMS(ListenMode)

    bool isConnected() const override;
    QString version() const override;
    QString currentCall() const override;
    int batteryLevel() const override;

    void connectDevice() override;
    void disconnectDevice() override;

    QString deviceID() const override;
    QStringList defaultCommandFiles() const override;

    ListenMode listenMode() const;
    void setListenMode(const ListenMode& listenMode);
    Q_SIGNAL void listenModeChanged();

    bool micsSwapped() const;
    Q_SIGNAL void micsSwappedChanged();

    bool hasTilt() const;
    Q_SIGNAL void hasTiltChanged();

    bool canBalanceListening() const;
    Q_SIGNAL void canBalanceListeningChanged();

    bool tiltEnabled() const;
    Q_SIGNAL void tiltEnabledChanged();
    void setTiltMode(bool tiltState);

    void sendMessage(const QString &message) override;

    Q_INVOKABLE void checkOTA() override;
    bool hasAvailableOTA() override;
    QString otaVersion() override;
    Q_INVOKABLE void downloadOTAData() override;
    Q_INVOKABLE void setOTAData ( const QString& md5sum, const QByteArray& firmware ) override;
    bool hasOTAData() override;
    Q_INVOKABLE void startOTA() override;
private:
    class Private;
    Private* d;
};

#endif//BTDEVICEEARS_H
