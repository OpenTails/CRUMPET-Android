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

#include "BTDeviceCommandModel.h"

#include "BTDevice.h"
#include "BTDeviceModel.h"
#include "TailCommandModel.h"

class BTDeviceCommandModel::Private
{
public:
    Private() {}
    BTDeviceModel* deviceModel{nullptr};
    struct Entry {
        TailCommandModel::CommandInfo* command{nullptr};
        QList<BTDevice*> devices;
    };
    QVector<Entry*> commands;
};

BTDeviceCommandModel::BTDeviceCommandModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
}

BTDeviceCommandModel::~BTDeviceCommandModel()
{
    delete d;
}

QHash<int, QByteArray> BTDeviceCommandModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {Name, "name"},
        {Command, "command"},
        {IsRunning, "isRunning"},
        {Category, "category"},
        {Duration, "duration"},
        {MinimumCooldown, "minimumCooldown"},
        {CommandIndex, "commandIndex"},
        {DeviceIDs, "deviceIDs"}
    };
    return roles;
}

QVariant BTDeviceCommandModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    if (checkIndex(index)) {
        Private::Entry* entry = d->commands[index.row()];
        switch(role) {
            case Name:
                result.setValue(entry->command->name);
                break;
            case Command:
                result.setValue(entry->command->command);
                break;
            case IsRunning:
                result.setValue(entry->command->isRunning);
                break;
            case Category:
                result.setValue(entry->command->category);
                break;
            case Duration:
                result.setValue(entry->command->duration);
                break;
            case MinimumCooldown:
                result.setValue(entry->command->minimumCooldown);
                break;
            case CommandIndex:
                result.setValue(index.row());
                break;
            case DeviceIDs:
            {
                QStringList ids;
                for (BTDevice* device : entry->devices) {
                    ids << device->deviceID;
                }
                result.setValue<QStringList>(ids);
                break;
            }
            default:
                break;
        }
    }
    return result;
}

int BTDeviceCommandModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->commands.count();
}

void BTDeviceCommandModel::setDeviceModel(BTDeviceModel* deviceModel)
{
    d->deviceModel = deviceModel;
}
