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

#include "CommandQueue.h"
#include "BTConnectionManager.h"
#include "BTDeviceCommandModel.h"
#include "BTDeviceModel.h"
#include "BTDevice.h"

#include <QTimer>

class CommandQueue::Private
{
public:
    Private(CommandQueue* qq, BTConnectionManager* connectionManager)
        : q(qq)
        , connectionManager(connectionManager)
    {
        currentCommandTimer = new QTimer(qq);
        currentCommandTimer->setSingleShot(true);
        currentCommandTimerChecker = new QTimer(qq);
        currentCommandTimerChecker->setInterval(100);
        q->connect(currentCommandTimer, &QTimer::timeout, [this](){
            currentCommandTimerChecker->stop();
            emit q->currentCommandRemainingMSecondsChanged(0);
        });
    }
    ~Private() {}

    CommandQueue* q{nullptr};

    struct Entry {
        Entry(const CommandInfo& command)
            : command(command)
        {}
        ~Entry() { }
        CommandInfo command;
        QStringList deviceIDs;
    };
    QVector<Entry*> commands;
    BTConnectionManager* connectionManager{nullptr};

    QTimer* popTimer{nullptr};
    QTimer* currentCommandTimer{nullptr};
    QTimer* currentCommandTimerChecker{nullptr};

    void pop()
    {
        // only pop if there's commands left
        if(commands.count() > 0) {
            Entry* entry = commands.takeFirst();

            // Command can be empty if it's a pause (possibly others as well,
            // though not yet, but just never send an empty command)
            if(!entry->command.command.isEmpty()) {
                connectionManager->sendMessage(entry->command.command, entry->deviceIDs);
                currentCommandTimer->setInterval(entry->command.duration + entry->command.minimumCooldown);
                currentCommandTimer->start();
                currentCommandTimerChecker->start();
                emit q->currentCommandTotalDurationChanged(currentCommandTimer->interval());
                emit q->currentCommandRemainingMSecondsChanged(currentCommandTimer->remainingTime());
            }

            popTimer->start(entry->command.duration + entry->command.minimumCooldown);

            emit q->countChanged(q->count());
            delete entry;
        }
    }
};

CommandQueue::CommandQueue(BTConnectionManager* connectionManager)
    : CommandQueueProxySource(connectionManager)
    , d(new Private(this, connectionManager))
{
    d->popTimer = new QTimer(this);
    d->popTimer->setSingleShot(true);
    connect(d->popTimer, &QTimer::timeout, this, [this](){ d->pop(); });

    connect(d->currentCommandTimerChecker, &QTimer::timeout, [this](){
        emit currentCommandRemainingMSecondsChanged(d->currentCommandTimer->remainingTime());
    });
}

CommandQueue::~CommandQueue()
{
    delete d;
}

QHash<int, QByteArray> CommandQueue::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {Name, "name"},
        {Command, "command"},
        {IsRunning, "isRunning"},
        {Category, "category"},
        {Duration, "duration"},
        {MinimumCooldown, "minimumCooldown"}
    };
    return roles;
}

QVariant CommandQueue::data(const QModelIndex& index, int role) const
{
    QVariant value;
    if(index.isValid() && index.row() > -1 && index.row() < d->commands.count()) {
        Private::Entry* entry = d->commands.at(index.row());
        switch(role) {
            case Name:
                value = entry->command.name;
                break;
            case Command:
                value = entry->command.command;
                break;
            case IsRunning:
                value = entry->command.isRunning;
                break;
            case Category:
                value = entry->command.category;
                break;
            case Duration:
                value = entry->command.duration;
                break;
            case MinimumCooldown:
                value = entry->command.minimumCooldown;
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

int CommandQueue::currentCommandRemainingMSeconds() const
{
    return d->currentCommandTimer->remainingTime();
}

int CommandQueue::currentCommandTotalDuration() const
{
    return d->currentCommandTimer->interval();
}

void CommandQueue::clear(const QString& deviceID)
{
    // Before doing anything else, ensure the timer doesn't suddenly pick stuff
    // out from underneath us. Stop all functions and let's do the thing.
    d->popTimer->stop();
    if (deviceID.isEmpty()) {
        d->commands.clear();
    } else {
        // Remove the command, but only if the command is requested for only that device
        // If the command is requested for other devices as well, remove this device from the list of requesting devices
    }
    emit countChanged(count());
}

void CommandQueue::pushPause(int durationMilliseconds, QStringList devices)
{
    qDebug() << "Adding a pause to the queue of" << durationMilliseconds << "milliseconds";
    CommandInfo command;
    command.name = "Pause";
    command.duration = durationMilliseconds;

    Private::Entry* entry = new Private::Entry(command);
    entry->deviceIDs = devices;
    d->commands.append(entry);
    emit countChanged(count());

    // If we have just pushed a command and the timer is not currently running,
    // let's fire one off now!
    if(!d->popTimer->isActive()) {
        d->pop();
    }
}

void CommandQueue::pushCommand(QString tailCommand, QStringList devices)
{
    qDebug() << Q_FUNC_INFO << tailCommand;
    const CommandInfo& command = qobject_cast<BTDeviceCommandModel*>(d->connectionManager->commandModel())->getCommand(tailCommand);
    qDebug() << "Command to push" << command.command;
    if(!command.isValid()) {
        return;
    }
    Private::Entry* entry = new Private::Entry(command);
    entry->deviceIDs = devices;
    d->commands.append(entry);
    emit countChanged(count());

    // If we have just pushed a command and the timer is not currently running,
    // let's fire one off now!
    if(!d->popTimer->isActive()) {
        d->pop();
    }
}

void CommandQueue::pushCommands(CommandInfoList commands, QStringList devices)
{
    if(commands.count() > 0) {
        for (const CommandInfo& command : commands) {
            Private::Entry* entry = new Private::Entry(command);
            entry->deviceIDs = devices;
            d->commands.append(entry);
        }
        emit countChanged(count());

        // If we have just pushed some commands and the timer is not currently
        // running, let's fire one off now!
        if(!d->popTimer->isActive()) {
            d->pop();
        }
    }
}

void CommandQueue::pushCommands(QStringList commands, QStringList devices)
{
    qDebug() << commands;
    for (auto command : commands) {
        if(command.startsWith("pause")) {
            QStringList pauseCommand = command.split(':');
            if(pauseCommand.count() == 2) {
                pushPause(pauseCommand[1].toInt() * 1000, devices);
            }
        } else {
            pushCommand(command, devices);
        }
    }
}

void CommandQueue::removeEntry(int index)
{
    d->commands.removeAt(index);
    emit countChanged(count());
}

void CommandQueue::swapEntries(int swapThis, int withThis)
{
    if(swapThis >= 0 && swapThis < d->commands.count() && withThis >= 0 && withThis < d->commands.count()) {
        Private::Entry* with = d->commands.takeAt(withThis);
        Private::Entry* swap = d->commands.takeAt(swapThis);
        d->commands.insert(swapThis, with);
        d->commands.insert(withThis, swap);
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
