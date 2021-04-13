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

#include "TailCommandModel.h"

#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>

class TailCommandModel::Private
{
public:
    Private() {}
    ~Private() {}

    CommandInfoList commands;
    QHash<QString,QTimer*> commandDeactivators;
};

TailCommandModel::TailCommandModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{}

TailCommandModel::~TailCommandModel()
{
    delete d;
}

QHash< int, QByteArray > TailCommandModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {Name, "name"},
        {Command, "command"},
        {IsRunning, "isRunning"},
        {Category, "category"},
        {Duration, "duration"},
        {MinimumCooldown, "minimumCooldown"},
        {CommandIndex, "commandIndex"},
        {IsAvailable, "isAvailable"}};
    return roles;
}

QVariant TailCommandModel::data(const QModelIndex& index, int role) const
{
    QVariant value;
    if(index.isValid() && index.row() > -1 && index.row() < d->commands.count()) {
        const CommandInfo& command = d->commands.at(index.row());
        switch(role) {
            case Name:
                value = command.name;
                break;
            case Command:
                value = command.command;
                break;
            case IsRunning:
                value = command.isRunning;
                break;
            case Category:
                value = command.category;
                break;
            case Duration:
                value = command.duration;
                break;
            case MinimumCooldown:
                value = command.minimumCooldown;
                break;
            case CommandIndex:
                value = index.row();
                break;
            case IsAvailable:
                value = command.isAvailable;
                break;
            default:
                break;
        }
    };
    return value;
}

int TailCommandModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return d->commands.count();
}

void TailCommandModel::clear()
{
    beginResetModel();
    d->commands.clear();
    endResetModel();
}

void TailCommandModel::addCommand(const CommandInfo& command)
{
    beginInsertRows(QModelIndex(), 0, 0);
    d->commands.insert(0, command);
    emit commandAdded(command);
    endInsertRows();
}

void TailCommandModel::removeCommand(const CommandInfo& command)
{
    int idx{0};
    bool found{false};
    for (const CommandInfo& other : d->commands) {
        if (command.compare(other)) {
            found = true;
            break;
        }
        ++idx;
    }
    if (found) {
        beginRemoveRows(QModelIndex(), idx, idx);
        emit commandRemoved(command);
        d->commands.removeAt(idx);
        endRemoveRows();
    }
}

void TailCommandModel::setRunning(const QString& command, bool isRunning)
{
//     qDebug() << "Command changing running state" << command << "being set to" << isRunning;
    int i = -1;
    for (CommandInfo& theCommand : d->commands) {
        ++i;
//         qDebug() << "Checking" << theCommand->command << "named" << theCommand->name;
        if(theCommand.command == command) {
//             qDebug() << "Found matching command!" << i;
            if(theCommand.isRunning != isRunning) {
//                 qDebug() << "Changing state";
                QModelIndex idx = index(i, 0);
                theCommand.isRunning = isRunning;
                theCommand.isAvailable = !isRunning;
                dataChanged(idx, idx, QVector<int>() << TailCommandModel::IsRunning << TailCommandModel::IsAvailable);

                int i2 = -1;
                for (CommandInfo& otherCommand : d->commands) {
                    ++i2;
                    if (i != i2 && otherCommand.group == theCommand.group) {
                        otherCommand.isAvailable = !isRunning;
                        QModelIndex idx2 = index(i2, 0);
                        dataChanged(idx2, idx2, QVector<int>() << TailCommandModel::IsAvailable);
                    }
                }
            }
            if (isRunning) {
                // Hackery hacky time - if we end up running for longer than we're supposed to,
                // assume that we missed something, and set ourselves available again.
                // Also, we may end up in a situation where we have, in fact, been activated
                // and deactivated correctly. In that case, let's not deactivate things we need
                // to keep active. This could be done above, but keeping the code together feels
                // simpler for future maintenance.
                QTimer *timer = d->commandDeactivators[theCommand.command];
                if (!timer) {
                    timer = new QTimer(this);
                    d->commandDeactivators[theCommand.command] = timer;
                    timer->setSingleShot(true);
                    timer->setInterval(theCommand.duration + theCommand.minimumCooldown);
                    connect(timer, &QTimer::timeout, this, [this,&theCommand,timer]() {
                        if (theCommand.isRunning) {
                            qDebug() << "Automatically deactivating the following command - for some reason we seem to have missed the device ending the command." << theCommand.command;
                            setRunning(theCommand.command, false);
                        }
                        d->commandDeactivators.remove(theCommand.command);
                        timer->deleteLater();
                    } );
                }
                timer->start();
            }
            break;
        }
    }
//     qDebug() << "Done changing command running state";
}

const CommandInfoList& TailCommandModel::allCommands() const
{
    return d->commands;
}

bool TailCommandModel::isRunning(const CommandInfo& cmd) const
{
    for (const CommandInfo& ourCmd : d->commands) {
        if (cmd.compare(ourCmd)) {
            return ourCmd.isRunning;
        }
    }
    return false;
}

bool TailCommandModel::isAvailable(const CommandInfo& cmd) const
{
    bool retVal{false};
    if (cmd.command == "TAILHM") {
        retVal = true;
    } else {
        for (const CommandInfo& ourCmd : d->commands) {
            if (cmd.compare(ourCmd)) {
                return ourCmd.isAvailable;
            }
        }
    }
    return retVal;
}
