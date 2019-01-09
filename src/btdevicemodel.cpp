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

#include "btdevicemodel.h"

class BTDeviceModel::Private
{
public:
    Private() {}
    ~Private() {}

    QList<BTDeviceModel::Device*> devices;
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
        Device* device = d->devices.at(index.row());
        switch(role) {
            case Name:
                value = device->name;
                break;
            case DeviceID:
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

void BTDeviceModel::addDevice(Device* newDevice)
{
    // It feels a little dirty to do it this way...
    if(newDevice->name == QLatin1String("(!)Tail1")) {
        for(const BTDeviceModel::Device* device : d->devices) {
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

void BTDeviceModel::removeDevice(Device* device)
{
    int idx = d->devices.indexOf(device);
    if (idx > -1) {
        beginRemoveRows(QModelIndex(), idx, idx);
        d->devices.removeAt(idx);
        emit countChanged();
        endRemoveRows();
    }
    delete device;
}

int BTDeviceModel::count()
{
    return d->devices.count();
}

const BTDeviceModel::Device* BTDeviceModel::getDevice(const QString& deviceID) const
{
    for(const BTDeviceModel::Device* device : d->devices) {
        if(device->deviceID == deviceID) {
            return device;
        }
    }
    return nullptr;
}

QString BTDeviceModel::getDeviceID(int deviceIndex) const
{
    if(deviceIndex >= 0 && deviceIndex < d->devices.count()) {
        return d->devices.at(deviceIndex)->deviceID;
    }
    return QLatin1String();
}
