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

#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractListModel>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QLowEnergyController>
#include <QTimer>

class AppSettings;
class GearBase;

class DeviceModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * Whether or not any device in the model is connected
     */
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
public:
    explicit DeviceModel (QObject* parent = nullptr);
    ~DeviceModel() override;

    enum Roles {
        Name = Qt::UserRole + 1, // 257
        DeviceID,                // 258
        DeviceVersion,           // 259
        BatteryLevel,            // 260
        CurrentCall,             // 261
        IsConnected,             // 262
        ActiveCommandTitles,     // 263
        Checked,                 // 264
        HasListening,            // 265
        ListeningState,          // 266 - either 0 (off), 1 (on in iOS mode), or 2 (full feedback mode)
        EnabledCommandsFiles,    // 267 - a stringlist
        MicsSwapped,             // 268 - boolean value for whether or not the microphones have been swapped
        SupportsOTA,             // 269 - boolean value for whether or not the device supports over the air updates
        HasAvailableOTA,         // 270 - boolean value for whether or not there is an update available for the device
        HasOTAData,              // 271 - boolean value for whether or not firmware has been downloaded and is ready to be installed
        DeviceProgress,          // 272 - integer from -1 through 100 (-1 meaning nothing ongoing, 0 meaning unknown progress, 1 through 100 being a percentage)
        ProgressDescription,     // 273 - string describing the current step for the device progress
        OperationInProgress,     // 274 - boolean which is true when the DeviceProgress role's value is larger than -1
        OTAVersion,              // 275 - string containing the available OTA version
        HasLights,               // 276 - boolean value for whether or not the gear has support for lights
        HasShutdown,             // 277 - boolean value for whether or not the gear supports the SHUTDOWN command
        HasNoPhoneMode,          // 278 - boolean value for whether or not the gear supports no phone mode
        NoPhoneModeGroups,       // 279 - variantlist where the key is the ID of a group, and the value is the human readable group names of that group
        ChargingState,           // 280 - integer describing what charging state the device is in (0 is not plugged in, 1 is charging, 2 is plugged in and fully charged)
        BatteryLevelPercent,     // 281 - integer between 0 and 100 representing the current battery level
        HasTilt,                 // 282 - boolean value for whether the device has tilt detection
        CanBalanceListening,     // 283 - boolean value for whether the device is able to be re-balanced
        TiltEnabled,             // 284 - boolean value for whether the device currently has tilt mode enabled
        KnownFirmwareMessage,    // 285 - a string which, if not empty, contains a description of the current firmware version, and why the device should be updated
        GestureEventValues,      // 286 - a list of integers corresponding to GearBase::GearGestureEvent values
        GestureEventTitles,      // 287 - a list of strings with human-readable names for GearBase::GearGestureEvent values
        GestureEventCommands,    // 288 - a string with the command set for the GearBase::GearGestureEvent with the equivalent index in GestureEventIndices
        GestureEventDevices,     // 289 - a list of strings with the target devices set for the GearBase::GearGestureEvent with the equivalent index in GestureEventIndices
        SupportedTiltEvents,     // 290 - the list of supported Tilt related events (a list of GearBase::GearGestureEvent values)
        SupportedSoundEvents,    // 291 - the list of supported Sound related events (a list of GearBase::GearGestureEvent values)
        Color,                   // 292 - a colour value to match the device's ID
        DeviceType,              // 293 - A string name of the device's type (e.g. DiGITAIL, MiTail, EarGear, EarGear 2, MiTail Mini, FlutterWings
        DeviceIcon,              // 294 - A string containing the icon name for the device type
        IsConnecting,            // 295 - Whether the device we are currently attempting to establish a connection to the device
        AutoConnect,             // 296 - Whether the device should be connected to automatically
        IsKnown,                 // 297 - Whether the device is known (we recognise a device as "known" if we have ever connected to it
    };
    Q_ENUM(Roles)

    AppSettings* appSettings() const;
    void setAppSettings(AppSettings* appSettings);

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Whether or not any device in the model is connected
     * @return True if any device in the model is connected
     */
    bool isConnected() const;
    Q_SIGNAL void isConnectedChanged(bool isConnected);

    /**
     * Add a new device to the model.
     * The new device is added at the start of the unsorted model
     * The model takes ownership of the device, and deletion should not
     * be done manually.
     * @param newDevice The new device to show in the model
     */
    Q_SLOT void addDevice(const QBluetoothDeviceInfo& deviceInfo);
    void addDevice(GearBase* newDevice);
    /**
     * Remove a device from the model.
     * The entry will be deleted by this function, and you should not attempt to
     * use the instance afterwards.
     * If the device is not maintained by this model, it will still be deleted!
     */
    void removeDevice(GearBase* device);

    int count();
    Q_SIGNAL void countChanged();

    GearBase* getDevice(const QString& deviceID) const;
    GearBase* getDeviceById(int index) const;
    Q_INVOKABLE QString getDeviceID(int deviceIndex) const;

    /**
     * Send a message to the devices with the IDs in the given list. If
     * the list is empty, the message will be sent to all devies (don't worry
     * about connected status, no message will be sent to devices that are not
     * connected, that'd just be silly, and also not technically possible).
     * @param message The message to be sent out
     * @param deviceIDs A list of devices to send the message to (empty means all)
     */
    Q_SLOT void sendMessage(const QString& message, const QStringList& deviceIDs = QStringList());
    Q_SIGNAL void deviceMessage(const QString& deviceID, const QString& message);
    Q_SIGNAL void deviceBlockingMessage(const QString& title, const QString& message);

    Q_SIGNAL void deviceAdded(GearBase* device);
    Q_SIGNAL void deviceRemoved(GearBase* device);
    Q_SIGNAL void deviceConnected(GearBase* device);
    Q_SIGNAL void deviceDisconnected(GearBase* device);
private:
    class Private;
    Private* d;
};

#endif//DEVICEMODEL_H
