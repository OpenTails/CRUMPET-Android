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

#ifndef COMMANDQUEUE_H
#define COMMANDQUEUE_H

#include <QAbstractListModel>
#include "TailCommandModel.h"
#include "rep_CommandQueueProxy_source.h"

class BTConnectionManager;

/**
 * @brief The main move and light command interface for the tails
 *
 * The method through which commands should be sent to the tail. The idea is that
 * while such commands as battery status and versions should be sent directly,
 * a more playlist style of approach will want to be employed for the moves
 * and light commands. The command queue will allow for commands to be queued
 * up in a progressive manner, and then fired off either immediately the previous
 * one ended, or with a given pause before the next is launched. This ensures that
 * the tail will not likely end up with the kind of damage which might otherwise
 * occur if we allowed commands to simply be fired off without a cooldown period.
 */
class CommandQueue : public CommandQueueProxySource
{
    Q_OBJECT
public:
    explicit CommandQueue(BTConnectionManager* connectionManager);
    virtual ~CommandQueue();

    enum Roles {
        Name = Qt::UserRole + 1,
        Command,
        IsRunning,
        Category,
        Duration,
        MinimumCooldown
    };

    QHash< int, QByteArray > roleNames() const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int count() const override;

    /**
     * The number of remaining milliseconds of the most recently launched command
     * launched by the queue. If this is zero, consider no command running.
     * @note This also includes the mandatory pause of the command
     * @return The remaining runtime of the current command in milliseconds
     */
    int currentCommandRemainingMSeconds() const override;
    /**
     * The number of milliseconds the current command will run for.
     * @Note, this is not updated until the next command is launched, and not included for pauses.
     */
    int currentCommandTotalDuration() const override;
    /**
     * Clear the queue of all commands
     */
    Q_SLOT void clear() override;
    /**
     * Add a pause to the end of the queue
     *
     * @param durationMilliseconds The duration of the pause in milliseconds
     */
    Q_SLOT void pushPause(int durationMilliseconds) override;
    /**
     * Add a specific command to the end of the queue. If there are no commands
     * currently running, the command will be run immediately.
     *
     * @param tailCommand The command you wish to add to the queue
     */
    Q_SLOT void pushCommand(QString tailCommand) override;
    /**
     * A convenient way of adding a whole list of commands to the queue in one go.
     * As with adding a single command, if nothing is currently running, once the
     * list has been added, the first command in the list will be run.
     *
     * @param commands The list of commands to add to the queue
     */
    Q_SLOT void pushCommands(TailCommandModel::CommandInfoList commands);
    /**
     * A convenience slot which takes a list of commands, and the special pause command
     * (which is "pause:" followed by an integer number representing the number of seconds
     * the pause should last).
     * @param commands A list of commands
     */
    Q_SLOT void pushCommands(QStringList commands) override;
    /**
     * Remove a specific command from the queue
     *
     * @param index The index of the command to be removed
     */
    Q_SLOT void removeEntry(int index) override;
    /**
     * Swap the location of two commands in the queue
     *
     * @param swapThis The first command (which will end up at the original position of withThis)
     * @param withThis The second command (which will end up at the original position of swapThis)
     */
    Q_SLOT void swapEntries(int swapThis, int withThis) override;
    /**
     * A convenience method for swapping a command with the entry above it
     * (or, in other words, move the command one position up in the queue)
     *
     * @param index The index of the command to move up one position
     */
    Q_SLOT void moveEntryUp(int index) override;
    /**
     * A convenience method for swapping a command with the entry below it
     * (or, in other words, move the command one position down in the queue)
     *
     * @param index The index of the command to move down one position
     */
    Q_SLOT void moveEntryDown(int index) override;
private:
    class Private;
    Private* d;
};

#endif//COMMANDQUEUE_H

