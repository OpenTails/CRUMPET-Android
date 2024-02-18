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

#include "CommandModel.h"

#include "DeviceModel.h"
#include "CommandInfo.h"
#include "GearBase.h"

#include <QRandomGenerator>
#include <QMutableVectorIterator>

class CommandModel::Private
{
public:
    Private(CommandModel * qq) : q(qq) {}
    CommandModel * q{nullptr};
    DeviceModel * deviceModel{nullptr};
    struct Entry {
        Entry(const CommandInfo& command)
            : command(command)
        {}
        ~Entry() { }
        CommandInfo command;
        QList<GearBase*> devices;
    };
    QVector<Entry*> commands;

    // Update the duration of the given entry's contained command to be whatever is the longest duration for this command in all the entry's devices
    void updateEntryDurations(Entry *entry) {
        entry->command.duration = 0;
        entry->command.minimumCooldown = 0;
        for (GearBase* device : entry->devices) {
            const CommandInfoList deviceCommands = device->commandModel->allCommands();
            for (const CommandInfo &existing : deviceCommands) {
                if (entry->command.equivalent(existing)) {
                    if (entry->command.duration < existing.duration) {
                        entry->command.duration = existing.duration;
                        entry->command.minimumCooldown = existing.minimumCooldown;
                    }
                }
            }
        }
        const int index = commands.indexOf(entry);
        const QModelIndex modelIndex = q->index(index);
        q->dataChanged(modelIndex, modelIndex, QVector<int>{CommandModel::Duration, CommandModel::MinimumCooldown});
    }

    void addCommand(const CommandInfo& command, GearBase* device) {
        Entry* entry{nullptr};
        // check if command already exists in some entry
        for (Entry* existing : commands) {
            if (existing->command.equivalent(command)) {
                entry = existing;
                break;
            }
        }
        // if not, create a new entry and store the command in it
        if (!entry) {
            entry = new Entry(command);
            q->beginInsertRows(QModelIndex(), commands.count(), commands.count());
            commands << entry;
            q->endInsertRows();
        }
        // add device to entry (shouldn't really be possible for this to happen twice, but...)
        if (!entry->devices.contains(device)) {
            entry->devices << device;
        }
        updateEntryDurations(entry);
    }

    void removeCommand(const CommandInfo& command, GearBase* device) {
        Entry* entry{nullptr};
        // check if command exists
        for (Entry* existing : commands) {
            if (existing->command.equivalent(command)) {
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
            } else {
                updateEntryDurations(entry);
            }
        }
    }

    void addDeviceCommands(GearBase* device) {
        GearCommandModel* deviceCommands = device->commandModel;
        for (const CommandInfo& command : deviceCommands->allCommands()) {
            addCommand(command, device);
        }
    }

    void removeDeviceCommands(GearBase* device) {
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

    void deviceDataChanged(GearBase* device, const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector< int >& roles) {
        GearCommandModel* deviceCommands = device->commandModel;
        int first = topLeft.row();
        int last = bottomRight.row();
        int i = -1;
        for (const CommandInfo& cmd : deviceCommands->allCommands()) {
            ++i;
            if (i < first) { continue; }
            if (i > last) { break; }
            Entry* theEntry{nullptr};
            int entryIdx{-1};
            for (Entry* entry : commands) {
                ++entryIdx;
                if (entry->command.equivalent(cmd) && entry->devices.contains(device)) {
                    theEntry = entry;
//                     qDebug() << "Found the entry which has the same command" << theEntry;
                    break;
                }
            }
            if (theEntry) {
                QVector<int> theRoles = roles;
                QVector<int> ourRoles;
                if (roles.length() == 0) {
                    // this would want commenting back in when there's things other than IsRunning to worry about,
                    // such as the future IsAvailable field (for a less obstructive "you can't run this command
                    // now" method that is less app-blocky)
                    theRoles = q->roleNames().keys().toVector();
//                     theRoles << GearCommandModel::IsRunning;
                }
                for (int role : theRoles) {
//                     qDebug() << "The role which changed was" << role;
                    if (role == GearCommandModel::IsRunning) {
                        // we've got something we care about, let's deal with it
                        bool anyRunning{false};
                        for (GearBase* aDevice : theEntry->devices) {
                            anyRunning = aDevice->commandModel->isRunning(cmd);
                            if (anyRunning) {
                                break;
                            }
                        }
                        theEntry->command.isRunning = anyRunning;
                        ourRoles << CommandModel::IsRunning;
                    } else if (role == GearCommandModel::IsAvailable) {
                        // we've got something we care about, let's deal with it
                        bool anyAvailable{false};
                        for (GearBase* aDevice : theEntry->devices) {
                            if (aDevice->isConnected()) {
                                anyAvailable = aDevice->commandModel->isAvailable(cmd);
                                if (anyAvailable) {
                                    break;
                                }
                            }
                        }
                        theEntry->command.isAvailable = anyAvailable;
                        ourRoles << CommandModel::IsAvailable;
                    }
                }
                q->dataChanged(q->index(entryIdx), q->index(entryIdx), ourRoles);
            } else {
                qDebug() << "Something broke, and we got a data changed signal for something with no equivalent Entry..." << device << cmd.command;
            }
        }
    }

    void registerDevice(GearBase* device) {
        GearCommandModel* deviceCommands = device->commandModel;
        QObject::connect(deviceCommands, &GearCommandModel::commandAdded, q, [this, device](const CommandInfo& command){ addCommand(command, device); });
        QObject::connect(deviceCommands, &GearCommandModel::commandRemoved, q, [this, device](const CommandInfo& command){ removeCommand(command, device); });
        QObject::connect(deviceCommands, &QAbstractListModel::modelAboutToBeReset, q, [this, device](){ removeDeviceCommands(device); });
        QObject::connect(deviceCommands, &QAbstractListModel::modelReset, q, [this, device](){ removeDeviceCommands(device); addDeviceCommands(device); });
        QObject::connect(deviceCommands, &QAbstractItemModel::dataChanged, q, [this, device](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector< int >& roles){ deviceDataChanged(device, topLeft, bottomRight, roles); });
        QObject::connect(device, &QObject::destroyed, q, [this, device](){ removeDeviceCommands(device); });
        QObject::connect(device, &GearBase::isConnectedChanged, q, [this, device](){
            removeDeviceCommands(device);
            if (device->isConnected()) {
                addDeviceCommands(device);
            }
        });
    }

    void unregisterDevice(GearBase* device) {
        device->disconnect(q);
        removeDeviceCommands(device);
    }
};

CommandModel::CommandModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
}

CommandModel::~CommandModel()
{
    delete d;
}

QHash<int, QByteArray> CommandModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {Name, "name"},
        {Command, "command"},
        {IsRunning, "isRunning"},
        {Category, "category"},
        {Duration, "duration"},
        {MinimumCooldown, "minimumCooldown"},
        {CommandIndex, "commandIndex"},
        {DeviceIDs, "deviceIDs"},
        {IsAvailable, "isAvailable"}
    };
    return roles;
}

QVariant CommandModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    if (checkIndex(index)) {
        Private::Entry* entry = d->commands[index.row()];
        switch(role) {
            case Name:
                result.setValue(entry->command.name);
                break;
            case Command:
                result.setValue(entry->command.command);
                break;
            case IsRunning:
                result.setValue(entry->command.isRunning);
                break;
            case Category:
                result.setValue(entry->command.category);
                break;
            case Duration:
                result.setValue(entry->command.duration);
                break;
            case MinimumCooldown:
                result.setValue(entry->command.minimumCooldown);
                break;
            case CommandIndex:
                result.setValue(index.row());
                break;
            case IsAvailable:
                result.setValue(entry->command.isAvailable);
                break;
            case DeviceIDs:
            {
                QStringList deviceIDs;
                for (GearBase* device : entry->devices) {
                    deviceIDs << device->deviceID();
                }
                result.setValue(deviceIDs);
                break;
            }
            default:
                result.setValue(QString::fromUtf8("Unknown role"));
                break;
        }
    }
    return result;
}

int CommandModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->commands.count();
}

void CommandModel::setDeviceModel(DeviceModel * deviceModel)
{
    if (d->deviceModel) {
        d->deviceModel->disconnect(this);
    }
    d->deviceModel = deviceModel;
    connect(deviceModel, &DeviceModel::deviceAdded, this, [this](GearBase* device){ d->registerDevice(device); });
    connect(deviceModel, &DeviceModel::deviceRemoved, this, [this](GearBase* device){ d->unregisterDevice(device); });
    for (int i = 0; i < deviceModel->rowCount() ; ++i) {
        d->registerDevice(deviceModel->getDevice(deviceModel->getDeviceID(i)));
    }
}

CommandInfo CommandModel::getCommand(int index) const
{
    if(index >= 0 && index < d->commands.count()) {
        return d->commands[index]->command;
    }
    return CommandInfo{};
}

CommandInfo CommandModel::getCommand(QString command) const
{
    CommandInfo cmd;
    // Just in case - this is specifically for TAILHM, which must exist (and
    // preparing for others that are the same, but basically that - these are
    // commands which are technically invalid, but always available)
    cmd.command = command;
    for(Private::Entry* current : d->commands) {
        if(current->command.command == command) {
            cmd = current->command;
            break;
        }
    }
    return cmd;
}

CommandInfo CommandModel::getRandomCommand(QStringList includedCategories) const
{
    if(d->commands.count() > 0) {
        CommandInfoList pickFrom;
        for(Private::Entry* current : d->commands) {
            if(includedCategories.isEmpty() || includedCategories.contains(current->command.category)) {
                pickFrom << current->command;
            }
        }
        if (pickFrom.count() > 0) {
            return pickFrom.at(QRandomGenerator::global()->bounded(pickFrom.count()));
        }
        qWarning() << "We have no commands to pick from - maybe we should inform the user of this...";
    }
    return CommandInfo{};
}
