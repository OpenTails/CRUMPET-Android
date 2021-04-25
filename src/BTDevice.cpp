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
#include <QSettings>
#include <QTimer>

#include "AppSettings.h"
#include "CommandPersistence.h"

class BTDevice::Private {
public:
    Private() {}
    ~Private() {}

    bool checked{true};
    QString name;
    QStringList enabledCommandsFiles;
    BTDeviceModel* parentModel;
};

BTDevice::BTDevice(const QBluetoothDeviceInfo& info, BTDeviceModel* parent)
    : QObject(parent)
    , deviceInfo(info)
    , d(new Private)
{
    d->name = info.name();
    // Set the digitail name to something more friendly than (!)Tail1
    if (d->name == "(!)Tail1") {
        d->name = "DIGITAiL";
    }
    d->parentModel = parent;

    QTimer* timer = new QTimer(this);
    timer->setInterval(1);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this](){ Q_EMIT activeCommandTitlesChanged(activeCommandTitles()); });
    connect(commandModel, &QAbstractItemModel::dataChanged, this, [timer](const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector< int >& /*roles*/){ timer->start(); });

    QSettings settings;
    d->enabledCommandsFiles = settings.value(QString{"enabledCommandFiles-%1"}.arg(info.address().toString())).toStringList();
    connect(this, &BTDevice::enabledCommandsFilesChanged, this, [this](){
        // save command files back to settings
        QSettings settings;
        settings.setValue(QString{"enabledCommandFiles-%1"}.arg(deviceInfo.address().toString()), d->enabledCommandsFiles);
        settings.sync();
    });
    emit enabledCommandsFilesChanged(d->enabledCommandsFiles);
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
    if (enabled && !d->enabledCommandsFiles.contains(filename)) {
        d->enabledCommandsFiles.append(filename);
        emit enabledCommandsFilesChanged(d->enabledCommandsFiles);
        if (isConnected()) {
            reloadCommands();
        }
    }
    else if (!enabled && d->enabledCommandsFiles.contains(filename)) {
        d->enabledCommandsFiles.removeAll(filename);
        emit enabledCommandsFilesChanged(d->enabledCommandsFiles);
        if (isConnected()) {
            reloadCommands();
        }
    }
}

void BTDevice::reloadCommands() {
    commandModel->clear();
    commandShorthands.clear();
    QVariantMap commandFiles = d->parentModel->appSettings()->commandFiles();
    // If there are no enabled files, we'll load the default, so we don't end up with no commands at all
    QStringList enabledFiles = d->enabledCommandsFiles.count() > 0 ? d->enabledCommandsFiles : defaultCommandFiles();
    for (const QString& enabledFile : enabledFiles) {
        QVariantMap file = commandFiles[enabledFile].toMap();
        CommandPersistence persistence;
        persistence.deserialize(file[QLatin1String{"contents"}].toString());
        if (persistence.error().isEmpty()) {
            for (const CommandInfo &command : persistence.commands()) {
                commandModel->addCommand(command);
            }
            for (const CommandShorthand& shorthand : persistence.shorthands()) {
                commandShorthands[shorthand.command] = shorthand.expansion.join(QChar{';'});
            }
        }
        else {
            qWarning() << "Failure in loading the commands data for" << enabledFile << "with the error:" << persistence.error();
        }
    }
}

QStringList BTDevice::defaultCommandFiles() const
{
    return QStringList{QLatin1String{":/commands/digitail-builtin.crumpet"}};
}
