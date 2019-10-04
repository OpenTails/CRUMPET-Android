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

    CommandQueue* q;

    // These commands are owned by TailCommandModel, do not delete them
    CommandInfoList commands;
    BTConnectionManager* connectionManager;

    QTimer* popTimer;
    QTimer* currentCommandTimer;
    QTimer* currentCommandTimerChecker;

    void pop()
    {
        // only pop if there's commands left
        if(commands.count() > 0) {
            CommandInfo command = commands.takeFirst();

            // Command can be empty if it's a pause (possibly others as well,
            // though not yet, but just never send an empty command)
            if(!command.command.isEmpty()) {
                connectionManager->sendMessage(command.command, QStringList{});
                currentCommandTimer->setInterval(command.duration + command.minimumCooldown);
                currentCommandTimer->start();
                currentCommandTimerChecker->start();
                emit q->currentCommandTotalDurationChanged(currentCommandTimer->interval());
                emit q->currentCommandRemainingMSecondsChanged(currentCommandTimer->remainingTime());
            }

            popTimer->start(command.duration + command.minimumCooldown);

            emit q->countChanged(q->count());
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

void CommandQueue::pushPause(int durationMilliseconds)
{
    CommandInfo command;
    command.name = "Pause";
    command.duration = durationMilliseconds;

    d->commands.append(command);
    emit countChanged(count());

    // If we have just pushed a command and the timer is not currently running,
    // let's fire one off now!
    if(!d->popTimer->isActive()) {
        d->pop();
    }
}

void CommandQueue::pushCommand(QString tailCommand)
{
    qDebug() << tailCommand;
    const CommandInfo& command = qobject_cast<BTDeviceCommandModel*>(d->connectionManager->commandModel())->getCommand(tailCommand);
    qDebug() << "Command to push" << command.command;
    if(!command.isValid()) {
        return;
    }
    d->commands.append(command);
    emit countChanged(count());

    // If we have just pushed a command and the timer is not currently running,
    // let's fire one off now!
    if(!d->popTimer->isActive()) {
        d->pop();
    }
}

void CommandQueue::pushCommands(CommandInfoList commands)
{
    if(commands.count() > 0) {
        d->commands.append(commands);
        emit countChanged(count());

        // If we have just pushed some commands and the timer is not currently
        // running, let's fire one off now!
        if(!d->popTimer->isActive()) {
            d->pop();
        }
    }
}

void CommandQueue::pushCommands(QStringList commands)
{
    qDebug() << commands;
    for (auto command : commands) {
        if(command.startsWith("pause")) {
            QStringList pauseCommand = command.split(':');
            if(pauseCommand.count() == 2) {
                pushPause(pauseCommand[1].toInt() * 1000);
            }
        } else {
            pushCommand(command);
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
        const CommandInfo& with = d->commands.takeAt(withThis);
        const CommandInfo& swap = d->commands.takeAt(swapThis);
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
