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

#ifndef PERMISSIONSMANAGER_H
#define PERMISSIONSMANAGER_H

#include <QObject>

class PermissionsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasBluetoothPermissions READ hasBluetoothPermissions NOTIFY permissionsChanged)
public:
    explicit PermissionsManager(QObject* parent = nullptr);
    ~PermissionsManager() override;

    Q_INVOKABLE void requestPermission(const QString& permission);
    Q_INVOKABLE void requestPermissionNow(const QString& permission);
    Q_INVOKABLE bool hasPermission(const QString& permission) const;
    Q_SIGNAL void permissionsChanged();

    bool hasBluetoothPermissions() const;
    Q_INVOKABLE void requestBluetoothPermissions();
private:
    class Private;
    Private *d;
};

#endif//PERMISSIONSMANAGER_H

