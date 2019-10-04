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
#include <QRandomGenerator>

class BTDeviceCommandModel::Private
{
public:
    Private(BTDeviceCommandModel* qq) : q(qq) {}
    BTDeviceCommandModel* q;
    BTDeviceModel* deviceModel{nullptr};
    struct Entry {
        Entry(const TailCommandModel::CommandInfo& command)
            : command(new TailCommandModel::CommandInfo(command))
        {}
        ~Entry() {
            delete command;
        }
        TailCommandModel::CommandInfo* command{nullptr};
        QList<BTDevice*> devices;
    };
    QVector<Entry*> commands;

    void addCommand(TailCommandModel::CommandInfo* command, BTDevice* device) {
        Entry* entry{nullptr};
        // check if command already exists in some entry
        for (Entry* existing : commands) {
            if (existing->command->compare(*command)) {
                entry = existing;
                break;
            }
        }
        // if not, create a new entry and store the command in it
        if (!entry) {
            entry = new Entry(*command);
            q->beginInsertRows(QModelIndex(), commands.count(), commands.count());
            commands << entry;
            q->endInsertRows();
        }
        // add device to entry (shouldn't really be possible for this to happen twice, but...)
        if (!entry->devices.contains(device)) {
            entry->devices << device;
        }
    }

    void removeCommand(TailCommandModel::CommandInfo* command, BTDevice* device) {
        Entry* entry{nullptr};
        // check if command exists
        for (Entry* existing : commands) {
            if (existing->command == command) {
                entry = existing;
                break;
            }
        }
        // if command exists in some entry, remove device from it
        if (entry) {
            if (entry->devices.contains(device)) {
                entry->devices.removeAll(device);
            }
            // if there are no more devices in that command, remove the entry
            if (entry->devices.count() == 0) {
                const int position = commands.indexOf(entry);
                q->beginRemoveRows(QModelIndex(), position, position);
                commands.remove(position);
                q->endRemoveRows();
                delete entry;
            }
        }
    }

    void addDeviceCommands(BTDevice* device) {
        TailCommandModel* deviceCommands = device->commandModel;
        for (TailCommandModel::CommandInfo* command : deviceCommands->allCommands()) {
            addCommand(command, device);
        }
    }

    void removeDeviceCommands(BTDevice* device) {
        q->beginResetModel();
        QMutableVectorIterator<Entry*> it(commands);
        while (it.hasNext()) {
            it.next();
            // Remove mention of device if it exists in an entry
            if (it.value()->devices.contains(device)) {
                it.value()->devices.removeAll(device);
                // If entry is now empty of devices, remove and delete it
                if (it.value()->devices.count() == 0) {
                    delete it.value();
                    it.remove();
                }
            }
        }
        q->endResetModel();
    }

    void registerDevice(BTDevice* device) {
        TailCommandModel* deviceCommands = device->commandModel;
        QObject::connect(deviceCommands, &TailCommandModel::commandAdded, q, [this, device](TailCommandModel::CommandInfo* command){ addCommand(command, device); });
        QObject::connect(deviceCommands, &TailCommandModel::commandRemoved, q, [this, device](TailCommandModel::CommandInfo* command){ removeCommand(command, device); });
        QObject::connect(deviceCommands, &QAbstractListModel::modelAboutToBeReset, q, [this, device](){ removeDeviceCommands(device); });
        QObject::connect(deviceCommands, &QAbstractListModel::modelReset, q, [this, device](){ addDeviceCommands(device); });

    }

    void unregisterDevice(BTDevice* device) {
        device->disconnect(q);
        removeDeviceCommands(device);
    }
};

BTDeviceCommandModel::BTDeviceCommandModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
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
                QStringList deviceIDs;
                for (BTDevice* device : entry->devices) {
                    deviceIDs << device->deviceID();
                }
                result.setValue(deviceIDs);
                break;
            }
            default:
                result.setValue(QString{"Unknown role"});
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
    if (d->deviceModel) {
        d->deviceModel->disconnect(this);
    }
    d->deviceModel = deviceModel;
    connect(deviceModel, &BTDeviceModel::deviceAdded, this, [this](BTDevice* device){ d->registerDevice(device); });
    connect(deviceModel, &BTDeviceModel::deviceRemoved, this, [this](BTDevice* device){ d->unregisterDevice(device); });
    for (int i = 0; i < deviceModel->rowCount() ; ++i) {
        d->registerDevice(deviceModel->getDevice(deviceModel->getDeviceID(i)));
    }
}

TailCommandModel::CommandInfo * BTDeviceCommandModel::getCommand(int index) const
{
    if(index >= 0 && index < d->commands.count()) {
        return d->commands[index]->command;
    }
    return nullptr;
}

TailCommandModel::CommandInfo * BTDeviceCommandModel::getCommand(QString command) const
{
    TailCommandModel::CommandInfo* cmd(nullptr);
    for(Private::Entry* current : d->commands) {
        if(current->command->command == command) {
            cmd = current->command;
            break;
        }
    }
    return cmd;
}

TailCommandModel::CommandInfo * BTDeviceCommandModel::getRandomCommand(QStringList includedCategories) const
{
    if(d->commands.count() > 0) {
        TailCommandModel::CommandInfoList pickFrom;
        for(Private::Entry* current : d->commands) {
            if(includedCategories.isEmpty() || includedCategories.contains(current->command->category)) {
                pickFrom << current->command;
            }
        }
        return pickFrom.at(QRandomGenerator::global()->bounded(pickFrom.count()));
    }
    return nullptr;
}
