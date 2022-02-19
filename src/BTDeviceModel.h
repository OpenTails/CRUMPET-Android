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

#ifndef BTDEVICEMODEL_H
#define BTDEVICEMODEL_H

#include <QAbstractListModel>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QLowEnergyController>
#include <QTimer>

class AppSettings;
class BTDevice;

class BTDeviceModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * Whether or not any device in the model is connected
     */
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
public:
    explicit BTDeviceModel(QObject* parent = nullptr);
    ~BTDeviceModel() override;

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
    };

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
    void addDevice(BTDevice* newDevice);
    /**
     * Remove a device from the model.
     * The entry will be deleted by this function, and you should not attempt to
     * use the instance afterwards.
     * If the device is not maintained by this model, it will still be deleted!
     */
    void removeDevice(BTDevice* device);

    /**
     * Cause the device model to request an update in the UI for the
     * device with the passed ID.
     * @param deviceID The ID of the device for which to request an update
     */
    void updateItem(const QString& deviceID);

    int count();
    Q_SIGNAL void countChanged();

    BTDevice* getDevice(const QString& deviceID) const;
    BTDevice* getDeviceById(int index) const;
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

    Q_SIGNAL void deviceAdded(BTDevice* device);
    Q_SIGNAL void deviceRemoved(BTDevice* device);
    Q_SIGNAL void deviceConnected(BTDevice* device);
    Q_SIGNAL void deviceDisconnected(BTDevice* device);
private:
    class Private;
    Private* d;
};

#endif//BTDEVICEMODEL_H
