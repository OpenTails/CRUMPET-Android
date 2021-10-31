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
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
    Q_PROPERTY(QString currentCall READ currentCall NOTIFY currentCallChanged)
    Q_PROPERTY(QString activeCommandTitles READ activeCommandTitles NOTIFY activeCommandTitlesChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(QString deviceID READ deviceID CONSTANT)
    Q_PROPERTY(QStringList enabledCommandsFiles READ enabledCommandsFiles NOTIFY enabledCommandsFilesChanged)
    Q_PROPERTY(bool supportsOTA READ supportsOTA NOTIFY supportsOTAChanged)
    Q_PROPERTY(bool hasAvailableOTA READ hasAvailableOTA NOTIFY hasAvailableOTAChanged)
    Q_PROPERTY(QString otaVersion READ otaVersion NOTIFY hasAvailableOTAChanged);
    Q_PROPERTY(bool hasOTAData READ hasOTAData NOTIFY hasOTADataChanged)
    Q_PROPERTY(int deviceProgress READ deviceProgress NOTIFY deviceProgressChanged)
    Q_PROPERTY(QString progressDescription READ progressDescription NOTIFY progressDescriptionChanged)
public:
    explicit BTDevice(const QBluetoothDeviceInfo& info, BTDeviceModel* parent = nullptr);
    ~BTDevice() override;

    QBluetoothDeviceInfo deviceInfo;

    TailCommandModel* commandModel{new TailCommandModel(this)};
    QMap<QString, QString> commandShorthands;

    bool supportsOTA();
    void setSupportsOTA(bool supportsOTA);
    Q_SIGNAL void supportsOTAChanged();

    bool checked() const;
    void setChecked(bool checked);
    Q_SIGNAL void checkedChanged(bool checked);

    virtual bool isConnected() const = 0;
    Q_SIGNAL void isConnectedChanged(bool isConnected);

    virtual QString name() const;
    void setName(const QString& name);
    Q_SIGNAL void nameChanged(QString name);

    QStringList enabledCommandsFiles() const;
    Q_SIGNAL void enabledCommandsFilesChanged(QStringList enabledCommandsFiles);
    void setCommandsFileEnabledState(const QString& filename, bool enabled);

    void reloadCommands();
    virtual QStringList defaultCommandFiles() const;

    virtual QString version() const = 0;
    Q_SIGNAL void versionChanged(QString version);

    virtual QString currentCall() const = 0;
    Q_SIGNAL void currentCallChanged(QString currentCall);

    virtual QString activeCommandTitles() const;
    Q_SIGNAL void activeCommandTitlesChanged(QString activeCommandTitles);

    virtual int batteryLevel() const = 0;
    Q_SIGNAL void batteryLevelChanged(int batteryLevel);

    virtual void connectDevice() = 0;
    virtual void disconnectDevice() = 0;

    virtual QString deviceID() const = 0;

    virtual void sendMessage(const QString &message) = 0;

    Q_SIGNAL void deviceMessage(const QString& deviceID, const QString& message);

    Q_INVOKABLE virtual void checkOTA() {};
    Q_INVOKABLE virtual bool hasAvailableOTA() { return false; };
    Q_INVOKABLE virtual QString otaVersion() { return QString{}; };
    Q_SIGNAL void hasAvailableOTAChanged();
    Q_INVOKABLE virtual void downloadOTAData() {};
    Q_INVOKABLE virtual bool hasOTAData() { return false; }
    Q_SIGNAL void hasOTADataChanged();
    Q_INVOKABLE virtual void setOTAData(const QString &md5sum, const QByteArray &firmware) { Q_UNUSED(md5sum); Q_UNUSED(firmware); };
    Q_INVOKABLE virtual void startOTA() {};

    // A number from -1 to 100 (-1 meaning nothing ongoing, 0 meaning unknown progress, 1 through 100 being a percentage)
    int deviceProgress() const;
    Q_SIGNAL void deviceProgressChanged();
    void setDeviceProgress(int progress);
    QString progressDescription() const;
    Q_SIGNAL void progressDescriptionChanged();
    void setProgressDescription(const QString &progressDescription);
private:
    class Private;
    Private* d;
};

#endif//BTDEVICE_H
