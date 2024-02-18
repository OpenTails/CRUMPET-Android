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

#include "GearCommandModel.h"
#include "DeviceModel.h"

class GearBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)
    Q_PROPERTY(QString currentCall READ currentCall NOTIFY currentCallChanged)
    Q_PROPERTY(QString activeCommandTitles READ activeCommandTitles NOTIFY activeCommandTitlesChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(int batteryLevelPercent READ batteryLevelPercent NOTIFY batteryLevelPercentChanged)
    Q_PROPERTY(QString deviceID READ deviceID CONSTANT)
    Q_PROPERTY(QStringList enabledCommandsFiles READ enabledCommandsFiles NOTIFY enabledCommandsFilesChanged)
    Q_PROPERTY(bool supportsOTA READ supportsOTA NOTIFY supportsOTAChanged)
    Q_PROPERTY(bool hasAvailableOTA READ hasAvailableOTA NOTIFY hasAvailableOTAChanged)
    Q_PROPERTY(QString otaVersion READ otaVersion NOTIFY hasAvailableOTAChanged)
    Q_PROPERTY(bool hasOTAData READ hasOTAData NOTIFY hasOTADataChanged)
    Q_PROPERTY(int deviceProgress READ deviceProgress NOTIFY deviceProgressChanged)
    Q_PROPERTY(QString progressDescription READ progressDescription NOTIFY progressDescriptionChanged)
    Q_PROPERTY(bool hasLights READ hasLights NOTIFY hasLightsChanged)
    Q_PROPERTY(bool hasShutdown READ hasShutdown NOTIFY hasShutdownChanged)
    Q_PROPERTY(bool hasNoPhoneMode READ hasNoPhoneMode NOTIFY hasNoPhoneModeChanged)
    Q_PROPERTY(QVariantList noPhoneModeGroups READ noPhoneModeGroups NOTIFY noPhoneModeGroupsChanged)
    Q_PROPERTY(int chargingState READ chargingState NOTIFY chargingStateChanged)
    Q_PROPERTY(QString knownFirmwareMessage READ knownFirmwareMessage NOTIFY knownFirmwareMessageChanged)
public:
    explicit GearBase(const QBluetoothDeviceInfo& info, DeviceModel * parent = nullptr);
    ~GearBase() override;

    QBluetoothDeviceInfo deviceInfo;

    GearCommandModel* commandModel{new GearCommandModel(this)};
    QMap<QString, QString> commandShorthands;

    QColor color() const;
    void setColor(const QColor &color);
    Q_SIGNAL void colorChanged();

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

    int batteryLevelPercent() const;
    void setBatteryLevelPercent(int batteryLevelPercent);
    Q_SIGNAL void batteryLevelPercentChanged();

    virtual void connectDevice() = 0;
    virtual void disconnectDevice() = 0;

    virtual QString deviceID() const { return deviceInfo.address().toString(); };

    virtual void sendMessage(const QString &message) = 0;

    Q_SIGNAL void deviceMessage(const QString& deviceID, const QString& message);
    Q_SIGNAL void deviceBlockingMessage(const QString& title, const QString& message);

    Q_INVOKABLE virtual void checkOTA() {};
    Q_INVOKABLE virtual bool hasAvailableOTA() { return false; };
    Q_INVOKABLE virtual void setOtaVersion(const QString &version) { Q_UNUSED(version); }
    Q_INVOKABLE virtual QString otaVersion() { return version(); };
    const QString manuallyLoadedOtaVersion() { return QLatin1String{"Manually Picked"}; };
    Q_SIGNAL void hasAvailableOTAChanged();
    Q_INVOKABLE virtual void downloadOTAData() {};
    // As an alternative to downloading the OTA data from a remote server, this can be used to
    // implement a method for explicitly setting the data (whose md5sum will them be
    // trusted implicitly). This trust means access to the functionality should be
    // limited to developer-only.
    Q_INVOKABLE virtual void loadFirmwareFile(const QString &filename);
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

    enum GearSensorEvent {
        UnknownEvent = 0,
        TiltLeftEvent = 10,
        TiltRightEvent = 11,
        TiltForwardEvent = 12,
        TiltBackwardEvent = 13,
        TiltNeutralEvent = 14,
        SoundLeftLoudEvent = 20,
        SoundLeftQuietEvent = 21,
        SoundNeutralEvent = 22,
        SoundRightQuietEvent = 23,
        SoundRightLoudEvent = 24,
    };
    Q_ENUM(GearSensorEvent)
    Q_SIGNAL void gearSensorEvent(const GearSensorEvent &event);
    void setGearSensorCommand(const GearSensorEvent &event, const QStringList& targetDeviceIDs, const QString &command);
    QString gearSensorCommand(const GearSensorEvent &event) const;
    QStringList gearSensorTargetDevices(const GearSensorEvent &event);
    Q_SIGNAL void gearSensorCommandDetailsChanged();
    virtual QVariantList supportedTiltEvents() { return QVariantList{}; };
    Q_SIGNAL void supportedTiltEventsChanged();
    virtual QVariantList supportedSoundEvents() { return QVariantList{}; };
    Q_SIGNAL void supportedSoundEventsChanged();

    bool hasLights() const;
    void setHasLights(bool hasLights);
    Q_SIGNAL void hasLightsChanged();

    bool hasShutdown() const;
    void setHasShutdown(bool hasShutdown);
    Q_SIGNAL void hasShutdownChanged();

    bool hasNoPhoneMode() const;
    void setHasNoPhoneMode(bool hasNoPhoneMode);
    Q_SIGNAL void hasNoPhoneModeChanged();
    QVariantList noPhoneModeGroups() const;
    void setNoPhoneModeGroups(QVariantList noPhoneModeGroups);
    Q_SIGNAL void noPhoneModeGroupsChanged();

    int chargingState() const;
    void setChargingState(int chargingState);
    Q_SIGNAL void chargingStateChanged();

    QString knownFirmwareMessage() const;
    void setKnownFirmwareMessage(const QString& knownFirmwareMessage);
    Q_SIGNAL void knownFirmwareMessageChanged();
private:
    class Private;
    Private* d;
};

#endif//BTDEVICE_H
