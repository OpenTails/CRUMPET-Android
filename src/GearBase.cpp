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
#include <QCryptographicHash>
#include <QFile>
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
    bool autoConnect{false};
    bool isKnown{false};
    bool isConnecting{false};
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

    bool isLoading{false};
};

GearBase::GearBase(const QBluetoothDeviceInfo& info, DeviceModel * parent)
    : QObject(parent)
    , deviceInfo(info)
    , d(new Private(this))
{
    d->name = info.name();
    // Set the various device names to actually match the product name, instead of the bluetooth ID
    if (d->name == QLatin1String{"(!)Tail1"}) {
        d->name = QLatin1String{"DIGITAiL"};
    } else if (d->name == QLatin1String{"EG2"}) {
        d->name = QLatin1String{"EarGear 2"};
    } else if (d->name == QLatin1String{"mitail"}) {
        d->name = QLatin1String{"MiTail"};
    } else if (d->name == QLatin1String{"minitail"}) {
        d->name = QLatin1String{"MiTail Mini"};
    } else if (d->name == QLatin1String{"flutter"}) {
        d->name = QLatin1String{"FlutterWings"};
    }
    d->parentModel = parent;

    QTimer* timer = new QTimer(this);
    timer->setInterval(1);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this](){ Q_EMIT activeCommandTitlesChanged(activeCommandTitles()); });
    connect(commandModel, &QAbstractItemModel::dataChanged, this, [timer](const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector< int >& /*roles*/){ timer->start(); });

    d->load();
    connect(this, &GearBase::isKnownChanged, this, [this](){ d->save(); });
    connect(this, &GearBase::autoConnectChanged, this, [this](){ d->save(); });
    connect(this, &GearBase::enabledCommandsFilesChanged, this, [this](){ d->save(); });
    connect(this, &GearBase::nameChanged, this, [this](){ d->save(); });
    connect(this, &GearBase::gearSensorEvent, this, [this](const GearSensorEvent &event){ d->handleGearSensorEvent(event); });
}

GearBase::~GearBase()
{
    delete d;
}

void GearBase::forget()
{
    if (isConnected()) {
        disconnectDevice();
    }
    QSettings settings;
    settings.remove(QLatin1String("%1/known").arg(deviceID()));
    settings.remove(QString::fromUtf8("enabledCommandFiles-%1").arg(deviceID()));
    settings.beginGroup("Gear");
    QHashIterator<GearSensorEvent, GearSensorEventDetails> detailsIterator{d->gearSensorEvents};
    while(detailsIterator.hasNext()) {
        detailsIterator.next();
        const GearSensorEventDetails &details = detailsIterator.value();
        const QString commandKey = QString::fromUtf8("%1/%2/command").arg(deviceID()).arg(detailsIterator.key());
        const QString devicesKey = QString::fromUtf8("%1/%2/devices").arg(deviceID()).arg(detailsIterator.key());
        if (details.command.isEmpty()) {
            settings.remove(commandKey);
            settings.remove(devicesKey);
        }
    }
    settings.endGroup();
    settings.beginGroup("DeviceNameList");
    settings.remove(deviceID());
    settings.endGroup();
    settings.sync();
    deleteLater();
}

void GearBase::Private::load()
{
    isLoading = true;
    QSettings settings;
    const QString commandFilesKey = QLatin1String("enabledCommandFiles-%1").arg(q->deviceID());
    QStringList oldList = settings.value(QString::fromUtf8("enabledCommandFiles-%1").arg(q->deviceID())).toStringList();
    if (oldList.isEmpty() == false) {
        settings.remove(QString::fromUtf8("enabledCommandFiles-%1").arg(q->deviceID()));
        settings.beginGroup("Gear");
        settings.setValue(commandFilesKey, oldList);
        settings.endGroup();
        settings.sync();
    }
    q->setAutoConnect(settings.value(QLatin1String("%1/autoConnect").arg(q->deviceID()), autoConnect).toBool());
    q->setIsKnown(settings.value(QLatin1String("%1/known").arg(q->deviceID()), false).toBool());
    settings.beginGroup("Gear");
    enabledCommandsFiles = settings.value(commandFilesKey).toStringList();
    Q_EMIT q->enabledCommandsFilesChanged(enabledCommandsFiles);
    gearSensorEvents.clear();
    QMetaEnum gearSensorEventEnum = GearBase::staticMetaObject.enumerator(GearBase::staticMetaObject.indexOfEnumerator("GearSensorEvent"));
    for (int enumKey = 0; enumKey < gearSensorEventEnum.keyCount(); ++enumKey) {
        GearSensorEvent eventKey = static_cast<GearSensorEvent>(gearSensorEventEnum.value(enumKey));
        const QString detailsCommand = settings.value(QString::fromUtf8("%1/%2/command").arg(q->deviceID()).arg(eventKey)).toString();
        const QStringList detailsDevices = settings.value(QString::fromUtf8("%1/%2/devices").arg(q->deviceID()).arg(eventKey)).toStringList();
        gearSensorEvents[eventKey] = GearSensorEventDetails{detailsDevices, detailsCommand};
    }
    Q_EMIT q->gearSensorCommandDetailsChanged();
    settings.endGroup();
    // Device name
    settings.beginGroup("DeviceNameList");
    q->setName(settings.value(q->deviceID(), name).toString());
    settings.endGroup();
    isLoading = false;
}

void GearBase::Private::save()
{
    if (isLoading == false) {
        QSettings settings;
        settings.setValue(QLatin1String("%1/autoConnect").arg(q->deviceID()), autoConnect);
        if (isKnown) {
            settings.setValue(QLatin1String("%1/known").arg(q->deviceID()), true);
        } else {
            settings.remove(QLatin1String("%1/known").arg(q->deviceID()));
        }
        settings.setValue(QString::fromUtf8("enabledCommandFiles-%1").arg(q->deviceID()), enabledCommandsFiles);
        settings.beginGroup("Gear");
        QHashIterator<GearSensorEvent, GearSensorEventDetails> detailsIterator{gearSensorEvents};
        while(detailsIterator.hasNext()) {
            detailsIterator.next();
            const GearSensorEventDetails &details = detailsIterator.value();
            const QString commandKey = QString::fromUtf8("%1/%2/command").arg(q->deviceID()).arg(detailsIterator.key());
            const QString devicesKey = QString::fromUtf8("%1/%2/devices").arg(q->deviceID()).arg(detailsIterator.key());
            if (details.command.isEmpty()) {
                settings.remove(commandKey);
                settings.remove(devicesKey);
            } else {
                settings.setValue(commandKey, details.command);
                settings.setValue(devicesKey, details.targetDeviceIDs);
            }
        }
        settings.endGroup();
        settings.beginGroup("DeviceNameList");
        if (q->name().length() == 0) {
            settings.remove(q->deviceID());
        } else {
            settings.setValue(q->deviceID(), q->name());
        }
        settings.endGroup();
        settings.sync();
    }
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

bool GearBase::autoConnect() const
{
    return d->autoConnect;
}

void GearBase::setAutoConnect(const bool& autoConnect)
{
    if (d->autoConnect != autoConnect) {
        d->autoConnect = autoConnect;
        Q_EMIT autoConnectChanged(autoConnect);
    }
}

bool GearBase::isKnown() const
{
    return d->isKnown;
}

void GearBase::setIsKnown(const bool& isKnown)
{
    if (d->isKnown != isKnown) {
        d->isKnown = isKnown;
        Q_EMIT isKnownChanged();
    }
}

bool GearBase::isConnecting() const
{
    return d->isConnecting;
}

void GearBase::setIsConnecting(bool isConnecting)
{
    if (d->isConnecting != isConnecting) {
        d->isConnecting = isConnecting;
        Q_EMIT isConnectingChanged();
    }
}

QString GearBase::name() const
{
    return d->name;
}

void GearBase::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        Q_EMIT nameChanged(name);
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
            separator = QLatin1String{", "};
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
        Q_EMIT enabledCommandsFilesChanged(d->enabledCommandsFiles);
        if (isConnected()) {
            reloadCommands();
        }
    }
    else if (!enabled && d->enabledCommandsFiles.contains(filename)) {
        d->enabledCommandsFiles.removeAll(filename);
        Q_EMIT enabledCommandsFilesChanged(d->enabledCommandsFiles);
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
                commandShorthands[shorthand.command] = shorthand.expansion.join(QChar::fromLatin1(';'));
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

void GearBase::loadFirmwareFile(const QString& filename)
{
    QFile dataFile(QUrl(filename).toLocalFile());
    if (dataFile.exists()) {
        if (dataFile.open(QFile::ReadOnly)) {
            QByteArray firmwareData = dataFile.readAll();
            dataFile.close();
            QString calculatedSum = QString::fromUtf8(QCryptographicHash::hash(firmwareData, QCryptographicHash::Md5).toHex());
            setOtaVersion(manuallyLoadedOtaVersion());
            setOTAData(calculatedSum, firmwareData);
        } else {
            qDebug() << Q_FUNC_INFO << "Failed to open the firmware file for loading:" << dataFile.errorString();
        }
    } else {
        qDebug() << Q_FUNC_INFO << "Firmware file" << filename << "does not exist on disk";
    }
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
