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

#include "BTDevice.h"

#include <QCoreApplication>
#include <QTimer>

#include "AppSettings.h"

class BTDevice::Private {
public:
    Private() {}
    ~Private() {}

    bool checked{true};
    QString name;
    QStringList enabledCommandsFiles;
};

BTDevice::BTDevice(const QBluetoothDeviceInfo& info, BTDeviceModel* parent)
    : QObject(parent)
    , deviceInfo(info)
    , d(new Private)
{
    d->name = info.name();

    QTimer* timer = new QTimer(this);
    timer->setInterval(1);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this](){ Q_EMIT activeCommandTitlesChanged(activeCommandTitles()); });
    connect(commandModel, &QAbstractItemModel::dataChanged, this, [timer](const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector< int >& /*roles*/){ timer->start(); });
}

BTDevice::~BTDevice()
{
    delete d;
}

bool BTDevice::checked() const
{
    return d->checked;
}

void BTDevice::setChecked(bool checked)
{
    d->checked = checked;
    Q_EMIT checkedChanged(d->checked);
}

QString BTDevice::name() const
{
    return d->name;
}

void BTDevice::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged(name);
    }
}

QString BTDevice::activeCommandTitles() const
{
    QString titles;
    QString separator;
    for(const CommandInfo& command : commandModel->allCommands()) {
        if (command.isRunning) {
            titles += separator + command.name;
            separator = QString{", "};
        }
    }
    return titles;
}

QStringList BTDevice::enabledCommandsFiles() const
{
    return d->enabledCommandsFiles;
}

void BTDevice::setCommandsFileEnabledState(const QString& filename, bool enabled)
{
    qDebug() << Q_FUNC_INFO << filename << enabled;
    if (enabled && !d->enabledCommandsFiles.contains(filename)) {
        d->enabledCommandsFiles.append(filename);
        emit enabledCommandsFilesChanged(d->enabledCommandsFiles);
    }
    else if (!enabled && d->enabledCommandsFiles.contains(filename)) {
        d->enabledCommandsFiles.removeAll(filename);
        emit enabledCommandsFilesChanged(d->enabledCommandsFiles);
    }
}
