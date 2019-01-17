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

#include "commandqueue.h"
#include "btconnectionmanager.h"

class CommandQueue::Private
{
public:
    Private()
        : connectionManager(nullptr)
    {}
    ~Private() {}

    // These commands are owned by TailCommandModel, do not delete them
    TailCommandModel::CommandInfoList commands;
    BTConnectionManager* connectionManager;
};

CommandQueue::CommandQueue(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
}

CommandQueue::~CommandQueue()
{
    delete d;
}

QHash<int, QByteArray> CommandQueue::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[Command] = "command";
    roles[IsRunning] = "isRunning";
    roles[Category] = "category";
    roles[Duration] = "duration";
    roles[MinimumCooldown] = "minimumCooldown";
    return roles;
}

QVariant CommandQueue::data(const QModelIndex& index, int role) const
{
    QVariant value;
    if(index.isValid() && index.row() > -1 && index.row() < d->commands.count()) {
        TailCommandModel::CommandInfo* command = d->commands.at(index.row());
        switch(role) {
            case Name:
                value = command->name;
                break;
            case Command:
                value = command->command;
                break;
            case IsRunning:
                value = command->isRunning;
                break;
            case Category:
                value = command->category;
                break;
            case Duration:
                value = command->duration;
                break;
            case MinimumCooldown:
                value = command->minimumCooldown;
                break;
            default:
                break;
        }
    };
    return value;
}

int CommandQueue::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return d->commands.count();
}

int CommandQueue::count() const
{
    return d->commands.count();
}

void CommandQueue::clear()
{
    beginResetModel();
    d->commands.clear();
    endResetModel();
    emit countChanged();
}

void CommandQueue::pushCommand(TailCommandModel::CommandInfo* command)
{
    beginInsertRows(QModelIndex(), d->commands.count(), d->commands.count());
    d->commands.append(command);
    endInsertRows();
    emit countChanged();
}

void CommandQueue::pushCommands(TailCommandModel::CommandInfoList commands)
{
    if(commands.count() > 0) {
        beginInsertRows(QModelIndex(), d->commands.count(), d->commands.count() + commands.count() - 1);
        d->commands.append(commands);
        endInsertRows();
        emit countChanged();
    }
}

void CommandQueue::removeEntry(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    d->commands.removeAt(index);
    endRemoveRows();
    emit countChanged();
}

void CommandQueue::swapEntries(int swapThis, int withThis)
{
    if(swapThis >= 0 && swapThis < d->commands.count() && withThis >= 0 && withThis < d->commands.count()) {
        QModelIndex idx1 = createIndex(swapThis, 0);
        QModelIndex idx2 = createIndex(withThis, 0);
        TailCommandModel::CommandInfo* with = d->commands.takeAt(withThis);
        TailCommandModel::CommandInfo* swap = d->commands.takeAt(swapThis);
        d->commands.insert(swapThis, with);
        d->commands.insert(withThis, swap);
        dataChanged(idx1, idx1);
        dataChanged(idx2, idx2);
    }
}

void CommandQueue::moveEntryUp(int index)
{
    swapEntries(index, index + 1);
}

void CommandQueue::moveEntryDown(int index)
{
    swapEntries(index - 1, index);
}

QObject * CommandQueue::connectionManager() const
{
    return d->connectionManager;
}

void CommandQueue::setConnectionManager(QObject* connectionManager)
{
    d->connectionManager = qobject_cast<BTConnectionManager*>(connectionManager);
    emit connectionManagerChanged();
}
