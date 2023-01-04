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

#ifndef BTDEVICECOMMANDMODEL_H
#define BTDEVICECOMMANDMODEL_H

#include <QAbstractListModel>
#include "GearCommandModel.h"

class BTDeviceModel;
/**
 * An agregation model, which agregates all the command models for all connected
 * devices and presents all the available commands only once (duplicates are allowed
 * if they are in different categories).
 */
class BTDeviceCommandModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit BTDeviceCommandModel(QObject* parent = nullptr);
    ~BTDeviceCommandModel() override;

    enum Roles {
        Name = Qt::UserRole + 1,
        Command,
        IsRunning,
        Category,
        Duration,
        MinimumCooldown,
        CommandIndex,
        DeviceIDs,
        IsAvailable
    };

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Set the DeviceModel which contains the devies this model should present commands from
     */
    void setDeviceModel(BTDeviceModel* deviceModel);

    /**
     * Get the command at a specified index
     *
     * @param index The index of the command to fetch
     * @return A command if the index was valid, or null if not
     */
    Q_INVOKABLE CommandInfo getCommand(int index) const;
    /**
     * Get the command with the specified actual command
     *
     * @param command The command to fetch information for
     * @return The command info instance for the specified command, or null if none was found
     */
    Q_INVOKABLE CommandInfo getCommand(QString command) const;
    /**
     * Get a random command, picked from the currently available commands, limited
     * to commands with the category listed in includedCategories. If the list is
     * empty, any command will be listed.
     *
     * @param includedCategories A list of strings matching the categories
     * @return A random command matching one of the requested categories
     */
    Q_INVOKABLE CommandInfo getRandomCommand(QStringList includedCategories) const;
private:
    class Private;
    Private* d;
};

#endif//BTDEVICECOMMANDMODEL_H
