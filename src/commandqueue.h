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
#include "tailcommandmodel.h"

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
class CommandQueue : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* connectionManager READ connectionManager WRITE setConnectionManager NOTIFY connectionManagerChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit CommandQueue(QObject* parent = 0);
    virtual ~CommandQueue();

    enum Roles {
        Name = Qt::UserRole + 1,
        Command,
        IsRunning,
        Category,
        Duration,
        MinimumCooldown
    };

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int count() const;
    Q_SIGNAL void countChanged();

    /**
     * Clear the queue of all commands
     */
    void clear();
    /**
     * Add a pause to the end of the queue
     *
     * @param durationMilliseconds The duration of the pause in milliseconds
     */
    void pushPause(int durationMilliseconds);
    /**
     * Add a specific command to the end of the queue. If there are no commands
     * currently running, the command will be run immediately.
     *
     * @param command The command you wish to add to the queue
     */
    void pushCommand(TailCommandModel::CommandInfo* command);
    /**
     * A convenient way of adding a whole list of commands to the queue in one go.
     * As with adding a single command, if nothing is currently running, once the
     * list has been added, the first command in the list will be run.
     *
     * @param commands The list of commands to add to the queue
     */
    void pushCommands(TailCommandModel::CommandInfoList commands);
    /**
     * Remove a specific command from the queue
     *
     * @param index The index of the command to be removed
     */
    void removeEntry(int index);
    /**
     * Swap the location of two commands in the queue
     *
     * @param swapThis The first command (which will end up at the original position of withThis)
     * @param withThis The second command (which will end up at the original position of swapThis)
     */
    void swapEntries(int swapThis, int withThis);
    /**
     * A convenience method for swapping a command with the entry above it
     * (or, in other words, move the command one position up in the queue)
     *
     * @param index The index of the command to move up one position
     */
    void moveEntryUp(int index);
    /**
     * A convenience method for swapping a command with the entry below it
     * (or, in other words, move the command one position down in the queue)
     *
     * @param index The index of the command to move down one position
     */
    void moveEntryDown(int index);

    QObject* connectionManager() const;
    void setConnectionManager(QObject* connectionManager);
    Q_SIGNAL void connectionManagerChanged();
private:
    class Private;
    Private* d;
};

#endif//COMMANDQUEUE_H

