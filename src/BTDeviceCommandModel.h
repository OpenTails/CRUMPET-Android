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
        DeviceIDs
    };

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void setDeviceModel(BTDeviceModel* deviceModel);
private:
    class Private;
    Private* d;
};

#endif//BTDEVICECOMMANDMODEL_H
