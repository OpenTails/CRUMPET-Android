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

#include "GearBase.h"

#include <QCoreApplication>
#include <QColor>
#include <QSettings>
#include <QTimer>

#include "AppSettings.h"
#include "CommandPersistence.h"

struct GearSensorEventDetails {
public:
    GearSensorEventDetails() {}
    GearSensorEventDetails(const QStringList &targetDeviceIDs, const QString &command)
        : targetDeviceIDs(targetDeviceIDs)
        , command(command)
    {}
    QStringList targetDeviceIDs;
    QString command;
};

class GearBase::Private {
public:
    Private(GearBase *q)
        : q(q)
    {}
    ~Private() {}
    void load();
    void save();
    void handleGearSensorEvent(const GearSensorEvent &event);

    GearBase *q{nullptr};
    int batteryLevelPercent{100};
    QColor color;
    bool supportsOTA{false};
    bool checked{true};
    bool hasLights{false};
    bool hasShutdown{false};
    bool hasNoPhoneMode{false};
    QVariantList noPhoneModeGroups;
    int chargingState{0};
    QString knownFirmwareMessage;
    QString name;
    int deviceProgress{-1};
    QString progressDescription;
    QStringList enabledCommandsFiles;
    DeviceModel * parentModel{nullptr};
    QHash<GearBase::GearSensorEvent, GearSensorEventDetails> gearSensorEvents;
};

GearBase::GearBase(const QBluetoothDeviceInfo& info, DeviceModel * parent)
    : QObject(parent)
    , deviceInfo(info)
    , d(new Private(this))
{
    d->name = info.name();
    // Set the digitail name to something more friendly than (!)Tail1
    if (d->name == "(!)Tail1") {
        d->name = "DIGITAiL";
    // Set the eargear version 2 name to something a bit nicer as well
    } else if (d->name == "EG2") {
        d->name = "EarGear 2";
    // Also set the FlutterWings name to actually match the product name, instead of the bluetooth ID
    } else if (d->name == "flutter") {
        d->name = "FlutterWings";
    }
    d->parentModel = parent;

    QTimer* timer = new QTimer(this);
    timer->setInterval(1);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this](){ Q_EMIT activeCommandTitlesChanged(activeCommandTitles()); });
    connect(commandModel, &QAbstractItemModel::dataChanged, this, [timer](const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector< int >& /*roles*/){ timer->start(); });

    d->load();
    connect(this, &GearBase::enabledCommandsFilesChanged, this, [this](){ d->save(); });
    connect(this, &GearBase::gearSensorEvent, this, [this](const GearSensorEvent &event){ d->handleGearSensorEvent(event); });
}

GearBase::~GearBase()
{
    delete d;
}

void GearBase::Private::load()
{
    QSettings settings;
    const QString commandFilesKey = QString("%1/enabledCommandFiles");
    QStringList oldList = settings.value(QString{"enabledCommandFiles-%1"}.arg(q->deviceID())).toStringList();
    if (oldList.isEmpty() == false) {
        settings.remove(QString{"enabledCommandFiles-%1"}.arg(q->deviceID()));
        settings.beginGroup("Gear");
        settings.setValue(commandFilesKey, oldList);
        settings.endGroup();
        settings.sync();
    }
    settings.beginGroup("Gear");
    enabledCommandsFiles = settings.value(commandFilesKey).toStringList();
    Q_EMIT q->enabledCommandsFilesChanged(enabledCommandsFiles);
    gearSensorEvents.clear();
    QMetaEnum gearSensorEventEnum = GearBase::staticMetaObject.enumerator(GearBase::staticMetaObject.indexOfEnumerator("GearSensorEvent"));
    for (int enumKey = 0; enumKey < gearSensorEventEnum.keyCount(); ++enumKey) {
        GearSensorEvent eventKey = static_cast<GearSensorEvent>(gearSensorEventEnum.value(enumKey));
        const QString detailsCommand = settings.value(QString("%1/%2/command").arg(q->deviceID()).arg(eventKey)).toString();
        const QStringList detailsDevices = settings.value(QString("%1/%2/devices").arg(q->deviceID()).arg(eventKey)).toStringList();
        gearSensorEvents[eventKey] = GearSensorEventDetails{detailsDevices, detailsCommand};
    }
    Q_EMIT q->gearSensorCommandDetailsChanged();
    settings.endGroup();
}

void GearBase::Private::save()
{
    QSettings settings;
    settings.setValue(QString{"enabledCommandFiles-%1"}.arg(q->deviceID()), enabledCommandsFiles);
    settings.beginGroup("Gear");
    QHashIterator<GearSensorEvent, GearSensorEventDetails> detailsIterator{gearSensorEvents};
    while(detailsIterator.hasNext()) {
        detailsIterator.next();
        const GearSensorEventDetails &details = detailsIterator.value();
        const QString commandKey = QString("%1/%2/command").arg(q->deviceID()).arg(detailsIterator.key());
        const QString devicesKey = QString("%1/%2/devices").arg(q->deviceID()).arg(detailsIterator.key());
        if (details.command == "") {
            settings.remove(commandKey);
            settings.remove(devicesKey);
        } else {
            settings.setValue(commandKey, details.command);
            settings.setValue(devicesKey, details.targetDeviceIDs);
        }
    }
    settings.endGroup();
    settings.sync();

}

void GearBase::Private::handleGearSensorEvent(const GearSensorEvent& event)
{
    if (gearSensorEvents.contains(event)) {
        const GearSensorEventDetails &details = gearSensorEvents[event];
        parentModel->sendMessage(details.command, details.targetDeviceIDs);
    }
}

QColor GearBase::color() const
{
    return d->color;
}

void GearBase::setColor(const QColor& color)
{
    if (d->color != color) {
        d->color = color;
        Q_EMIT colorChanged();
    }
}

bool GearBase::supportsOTA()
{
    return d->supportsOTA;
}

void GearBase::setSupportsOTA(bool supportsOTA)
{
    d->supportsOTA = supportsOTA;
    Q_EMIT supportsOTAChanged();
}

bool GearBase::checked() const
{
    return d->checked;
}

void GearBase::setChecked(bool checked)
{
    d->checked = checked;
    Q_EMIT checkedChanged(d->checked);
}

QString GearBase::name() const
{
    return d->name;
}

void GearBase::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged(name);
    }
}

int GearBase::batteryLevelPercent() const
{
    return d->batteryLevelPercent;
}

void GearBase::setBatteryLevelPercent(int batteryLevelPercent)
{
    if (d->batteryLevelPercent != batteryLevelPercent) {
        d->batteryLevelPercent = batteryLevelPercent;
        Q_EMIT batteryLevelPercentChanged();
    }
}

QString GearBase::activeCommandTitles() const
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

QStringList GearBase::enabledCommandsFiles() const
{
    return d->enabledCommandsFiles;
}

void GearBase::setCommandsFileEnabledState(const QString& filename, bool enabled)
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

void GearBase::reloadCommands() {
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

QStringList GearBase::defaultCommandFiles() const
{
    return QStringList{QLatin1String{":/commands/digitail-builtin.crumpet"}};
}

int GearBase::deviceProgress() const
{
    return d->deviceProgress;
}

void GearBase::setDeviceProgress(int progress)
{
    if (d->deviceProgress != progress) {
        d->deviceProgress = progress;
        Q_EMIT deviceProgressChanged();
    }
}

QString GearBase::progressDescription() const
{
    return d->progressDescription;
}

void GearBase::setProgressDescription(const QString& progressDescription)
{
    if (d->progressDescription != progressDescription) {
        d->progressDescription = progressDescription;
        Q_EMIT progressDescriptionChanged();
    }
}

void GearBase::setGearSensorCommand(const GearSensorEvent& event, const QStringList& targetDeviceIDs, const QString& command)
{
    d->gearSensorEvents[event].command = command;
    d->gearSensorEvents[event].targetDeviceIDs = targetDeviceIDs;
    Q_EMIT gearSensorCommandDetailsChanged();
    d->save();
}

QString GearBase::gearSensorCommand(const GearSensorEvent& event) const
{
    if (d->gearSensorEvents.contains(event)) {
        return d->gearSensorEvents[event].command;
    }
    return QString{};
}

QStringList GearBase::gearSensorTargetDevices(const GearSensorEvent& event)
{
    if (d->gearSensorEvents.contains(event)) {
        return d->gearSensorEvents[event].targetDeviceIDs;
    }
    return QStringList{};
}

bool GearBase::hasLights() const
{
    return d->hasLights;
}

void GearBase::setHasLights(bool hasLights)
{
    if (d->hasLights != hasLights) {
        d->hasLights = hasLights;
        Q_EMIT hasLightsChanged();
    }
}

bool GearBase::hasShutdown() const
{
    return d->hasShutdown;
}

void GearBase::setHasShutdown(bool hasShutdown)
{
    if (d->hasShutdown != hasShutdown) {
        d->hasShutdown = hasShutdown;
        Q_EMIT hasShutdownChanged();
    }
}

bool GearBase::hasNoPhoneMode() const
{
    return d->hasNoPhoneMode;
}

void GearBase::setHasNoPhoneMode(bool hasNoPhoneMode)
{
    if (d->hasNoPhoneMode != hasNoPhoneMode) {
        d->hasNoPhoneMode = hasNoPhoneMode;
        Q_EMIT hasNoPhoneModeChanged();
    }
}

QVariantList GearBase::noPhoneModeGroups() const
{
    return d->noPhoneModeGroups;
}

void GearBase::setNoPhoneModeGroups(QVariantList noPhoneModeGroups)
{
    if (d->noPhoneModeGroups != noPhoneModeGroups) {
        d->noPhoneModeGroups = noPhoneModeGroups;
        Q_EMIT noPhoneModeGroupsChanged();
    }
}

int GearBase::chargingState() const
{
    return d->chargingState;
}

void GearBase::setChargingState(int chargingState)
{
    if (d->chargingState != chargingState) {
        d->chargingState = chargingState;
        Q_EMIT chargingStateChanged();
    }
}

QString GearBase::knownFirmwareMessage() const
{
    return d->knownFirmwareMessage;
}

void GearBase::setKnownFirmwareMessage(const QString &knownFirmwareMessage)
{
    if (d->knownFirmwareMessage != knownFirmwareMessage) {
        d->knownFirmwareMessage = knownFirmwareMessage;
        Q_EMIT knownFirmwareMessageChanged();
    }
}
