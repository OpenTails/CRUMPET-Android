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

    int batteryLevelPercent{100};
    bool supportsOTA{false};
    bool checked{true};
    bool hasLights{false};
    bool hasShutdown{false};
    bool hasNoPhoneMode{false};
    QVariantList noPhoneModeGroups;
    int chargingState{0};
    QString name;
    int deviceProgress{-1};
    QString progressDescription;
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
    } else if (d->name == "EG2") {
        d->name = "EarGear 2";
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

bool BTDevice::supportsOTA()
{
    return d->supportsOTA;
}

void BTDevice::setSupportsOTA(bool supportsOTA)
{
    d->supportsOTA = supportsOTA;
    Q_EMIT supportsOTAChanged();
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

int BTDevice::batteryLevelPercent() const
{
    return d->batteryLevelPercent;
}

void BTDevice::setBatteryLevelPercent(int batteryLevelPercent)
{
    if (d->batteryLevelPercent != batteryLevelPercent) {
        d->batteryLevelPercent = batteryLevelPercent;
        Q_EMIT batteryLevelPercentChanged();
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

int BTDevice::deviceProgress() const
{
    return d->deviceProgress;
}

void BTDevice::setDeviceProgress(int progress)
{
    if (d->deviceProgress != progress) {
        d->deviceProgress = progress;
        Q_EMIT deviceProgressChanged();
    }
}

QString BTDevice::progressDescription() const
{
    return d->progressDescription;
}

void BTDevice::setProgressDescription(const QString& progressDescription)
{
    if (d->progressDescription != progressDescription) {
        d->progressDescription = progressDescription;
        Q_EMIT progressDescriptionChanged();
    }
}

bool BTDevice::hasLights() const
{
    return d->hasLights;
}

void BTDevice::setHasLights(bool hasLights)
{
    if (d->hasLights != hasLights) {
        d->hasLights = hasLights;
        Q_EMIT hasLightsChanged();
    }
}

bool BTDevice::hasShutdown() const
{
    return d->hasShutdown;
}

void BTDevice::setHasShutdown(bool hasShutdown)
{
    if (d->hasShutdown != hasShutdown) {
        d->hasShutdown = hasShutdown;
        Q_EMIT hasShutdownChanged();
    }
}

bool BTDevice::hasNoPhoneMode() const
{
    return d->hasNoPhoneMode;
}

void BTDevice::setHasNoPhoneMode(bool hasNoPhoneMode)
{
    if (d->hasNoPhoneMode != hasNoPhoneMode) {
        d->hasNoPhoneMode = hasNoPhoneMode;
        Q_EMIT hasNoPhoneModeChanged();
    }
}

QVariantList BTDevice::noPhoneModeGroups() const
{
    return d->noPhoneModeGroups;
}

void BTDevice::setNoPhoneModeGroups(QVariantList noPhoneModeGroups)
{
    if (d->noPhoneModeGroups != noPhoneModeGroups) {
        d->noPhoneModeGroups = noPhoneModeGroups;
        Q_EMIT noPhoneModeGroupsChanged();
    }
}

int BTDevice::chargingState() const
{
    return d->chargingState;
}

void BTDevice::setChargingState(int chargingState)
{
    if (d->chargingState != chargingState) {
        d->chargingState = chargingState;
        Q_EMIT chargingStateChanged();
    }
}
