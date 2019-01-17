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

    void clear();
    void pushCommand(TailCommandModel::CommandInfo* command);
    void pushCommands(TailCommandModel::CommandInfoList commands);
    void removeEntry(int index);
    void swapEntries(int swapThis, int withThis);
    void moveEntryUp(int index);
    void moveEntryDown(int index);

    QObject* connectionManager() const;
    void setConnectionManager(QObject* connectionManager);
    Q_SIGNAL void connectionManagerChanged();
private:
    class Private;
    Private* d;
};

#endif//COMMANDQUEUE_H

