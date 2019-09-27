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

#include "BTDeviceModel.h"
#include "AppSettings.h"
#include "BTDevice.h"

class BTDeviceModel::Private
{
public:
    Private()
    {
        readDeviceNames();
    }
    ~Private() {}

    void readDeviceNames();

    AppSettings* appSettings = nullptr;
    QList<BTDevice*> devices;
    QMap<QString, QString> deviceNames;
};

BTDeviceModel::BTDeviceModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
}

BTDeviceModel::~BTDeviceModel()
{
    delete d;
}

void BTDeviceModel::Private::readDeviceNames()
{
    if (!appSettings) {
        return;
    }

    deviceNames.clear();

    const QVariantMap map = appSettings->deviceNames();

    for (QVariantMap::const_iterator it = map.cbegin(); it != map.cend(); ++it) {
        deviceNames[it.key()] = it.value().toString();
    }
}

AppSettings *BTDeviceModel::appSettings() const
{
    return d->appSettings;
}

void BTDeviceModel::setAppSettings(AppSettings *appSettings)
{
    d->appSettings = appSettings;
    d->readDeviceNames();
}

QHash< int, QByteArray > BTDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[DeviceID] = "deviceID";
    return roles;
}

QVariant BTDeviceModel::data(const QModelIndex& index, int role) const
{
    QVariant value;
    if(index.isValid() && index.row() > -1 && index.row() < d->devices.count()) {
        BTDevice* device = d->devices.at(index.row());
        switch(role) {
            case Name: {
                auto name = d->deviceNames.value(device->deviceInfo.address().toString());
                if (name.isEmpty()) {
                    value = device->name;
                } else {
                    value = name;
                }
                break;
            } case DeviceID:
                value = device->deviceID;
                break;
            default:
                break;
        }
    };
    return value;
}

int BTDeviceModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return d->devices.count();
}

void BTDeviceModel::addDevice(const QBluetoothDeviceInfo& deviceInfo)
{
    BTDevice* newDevice = new BTDevice(deviceInfo);
    connect(newDevice, &BTDevice::deviceMessage, this, &BTDeviceModel::deviceMessage);
    connect(newDevice, &QObject::destroyed, this, [this, newDevice](){
        int index = d->devices.indexOf(newDevice);
        if(index > -1) {
            beginRemoveRows(QModelIndex(), index, index);
            d->devices.removeAll(newDevice);
            endRemoveRows();
        }
    });
    // It feels a little dirty to do it this way...
    if(newDevice->name == QLatin1String("(!)Tail1")) {
        for(const BTDevice* device : d->devices) {
            if(device->deviceID == newDevice->deviceID) {
                // Don't add the same device twice. Thanks bt discovery. Thiscovery.
                return;
            }
        }

        beginInsertRows(QModelIndex(), 0, 0);
        d->devices.insert(0, newDevice);
        emit countChanged();
        endInsertRows();
    }
}

void BTDeviceModel::removeDevice(BTDevice* device)
{
    int idx = d->devices.indexOf(device);
    if (idx > -1) {
        beginRemoveRows(QModelIndex(), idx, idx);
        d->devices.removeAt(idx);
        emit countChanged();
        endRemoveRows();
    }
    device->deleteLater();
}

int BTDeviceModel::count()
{
    return d->devices.count();
}

BTDevice* BTDeviceModel::getDevice(const QString& deviceID) const
{
    for(BTDevice* device : d->devices) {
        if(device->deviceID == deviceID) {
            return device;
        }
    }
    return nullptr;
}

void BTDeviceModel::updateItem(const QString& deviceID)
{
    d->readDeviceNames();
    for(int idx = 0; idx < d->devices.count(); ++idx) {
        if(d->devices[idx]->deviceID == deviceID) {
            emit dataChanged(index(idx, 0), index(idx, 0));
        }
    }
}

QString BTDeviceModel::getDeviceID(int deviceIndex) const
{
    if(deviceIndex >= 0 && deviceIndex < d->devices.count()) {
        return d->devices.at(deviceIndex)->deviceID;
    }
    return QLatin1String();
}
