/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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

#include "AppSettings.h"
#include "AlarmList.h"
#include "Alarm.h"
#include "CommandPersistence.h"

#include <KLocalizedString>

#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QTimer>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

class AppSettings::Private
{
public:
    Private()
    {
        idleCategories << QLatin1String{"relaxed"};
    }
    ~Private() {}

    bool advancedMode = false;
    bool developerMode = false;
    bool idleMode = false;
    bool autoReconnect = true;
    bool alwaysSendToAll = false;
    QStringList idleCategories;
    int idleMinPause = 15;
    int idleMaxPause = 60;
    bool fakeTailMode = false;
    QString languageOverride;

    QMap<QString, QStringList> moveLists;
    QString activeMoveListName;

    AlarmList* alarmList = nullptr;
    QString activeAlarmName;

    QVariantMap commandFiles;
    bool isInitialized{false};
};

static const QLatin1Char semicolon{';'};
AppSettings::AppSettings(QObject* parent)
    : AppSettingsProxySource(parent)
    , d(new Private)
{
    QSettings settings;

    d->advancedMode = settings.value("advancedMode", d->advancedMode).toBool();
    d->developerMode = settings.value("developerMode", d->developerMode).toBool();
    d->idleMode = settings.value("idleMode", d->idleMode).toBool();
    d->autoReconnect = settings.value("autoReconnect", d->autoReconnect).toBool();
    d->alwaysSendToAll = settings.value("alwaysSendToAll", d->alwaysSendToAll).toBool();
    d->idleCategories = settings.value("idleCategories", d->idleCategories).toStringList();
    d->idleMinPause = settings.value("idleMinPause", d->idleMinPause).toInt();
    d->idleMaxPause = settings.value("idleMaxPause", d->idleMaxPause).toInt();
    d->fakeTailMode = settings.value("fakeTailMode", d->fakeTailMode).toBool();
    d->languageOverride = settings.value("languageOverride", d->languageOverride).toString();

    settings.beginGroup("MoveLists");
    QStringList moveLists = settings.allKeys();
    for(const QString& list : moveLists) {
        d->moveLists[list] = settings.value(list).toString().split(semicolon);
    }
    settings.endGroup();

    auto saveMoveLists = [this](){
        QSettings settings;
        settings.beginGroup("MoveLists");
        QMap<QString, QStringList>::const_iterator i = d->moveLists.constBegin();
        while(i != d->moveLists.constEnd()) {
            if(!i.key().isEmpty()) {
                settings.setValue(i.key(), i.value().join(semicolon));
            }
            ++i;
        }
        settings.endGroup();
    };

    connect(this, &AppSettings::moveListsChanged, saveMoveLists);
    connect(this, &AppSettings::moveListChanged, saveMoveLists);

    d->alarmList = new AlarmList(this);

    loadAlarmList();

    connect(d->alarmList, &AlarmList::listChanged, this, &AppSettings::onAlarmListChanged);
    connect(d->alarmList, &AlarmList::alarmExisted, this, &AppSettings::alarmExisted);
    connect(d->alarmList, &AlarmList::alarmNotExisted, this, &AppSettings::alarmNotExisted);

    const QStringList builtInCrumpets{QLatin1String{":/commands/eargear-base.crumpet"}, QLatin1String{":/commands/eargear2-base.crumpet"}, QLatin1String{":/commands/digitail-builtin.crumpet"}, QLatin1String{":/commands/mitail-builtin.crumpet"}, QLatin1String{":/commands/mitailmini-builtin.crumpet"}};
    for (const QString& filename : builtInCrumpets) {
        QString data;
        QFile file(filename);
        if(file.open(QIODevice::ReadOnly)) {
            data = QString::fromUtf8(file.readAll());
        }
        else {
            qWarning() << "Failed to open the included resource containing the basic command setup for your gear, this is very much not a good thing";
        }
        file.close();
        addCommandFile(filename, data);
        QVariantMap fileMap = d->commandFiles[filename].toMap();
        fileMap[QLatin1String{"isEditable"}] = false;
        d->commandFiles[filename] = fileMap;
    }
    settings.beginGroup("CrumpetFiles");
    for (const QString& filename : settings.childKeys()) {
        addCommandFile(filename, settings.value(filename).toString());
    }
    settings.endGroup();
    Q_EMIT commandFilesChanged(d->commandFiles);

    d->isInitialized = true;
}

AppSettings::~AppSettings()
{
    delete d;
}

int AppSettings::androidApiLevel() const
{
#ifdef Q_OS_ANDROID
    return QtAndroid::androidSdkVersion();
#else
    return 0;
#endif
}

bool AppSettings::advancedMode() const
{
    return d->advancedMode;
}

void AppSettings::setAdvancedMode(bool newValue)
{
    qDebug() << Q_FUNC_INFO << newValue;
    if(newValue != d->advancedMode) {
        d->advancedMode = newValue;
        QSettings settings;
        settings.setValue("advancedMode", d->advancedMode);
        Q_EMIT advancedModeChanged(newValue);
    }
}

bool AppSettings::developerMode() const
{
    return d->developerMode;
}

void AppSettings::setDeveloperMode(bool newValue)
{
    qDebug() << Q_FUNC_INFO << newValue;
    if(newValue != d->developerMode) {
        d->developerMode = newValue;
        QSettings settings;
        settings.setValue("developerMode", d->developerMode);
        Q_EMIT developerModeChanged(newValue);
    }
}

bool AppSettings::idleMode() const
{
    return d->idleMode;
}

void AppSettings::setIdleMode(bool newValue)
{
    qDebug() << Q_FUNC_INFO << newValue;
    static QTimer timer;

    if(newValue != d->idleMode) {
        timer.stop();

        d->idleMode = newValue;
        QSettings settings;
        settings.setValue("idleMode", d->idleMode);
        Q_EMIT idleModeChanged(newValue);

        if (d->idleMode) {
            timer.setSingleShot(true);
            // One hour - had it at four before, but safety is better
            timer.start(60 * 60 * 1000);

            connect(&timer, &QTimer::timeout, this, [this] {
                qDebug() << "The Casual Mode timeout was reached";
                setIdleMode(false);
                Q_EMIT idleModeTimeout();
            });
        }
    }
}

bool AppSettings::autoReconnect() const
{
    return d->autoReconnect;
}

void AppSettings::setAutoReconnect(bool newValue)
{
    qDebug() << Q_FUNC_INFO << newValue;
    if(newValue != d->autoReconnect) {
        d->autoReconnect = newValue;
        QSettings settings;
        settings.setValue("autoReconnect", d->autoReconnect);
        Q_EMIT autoReconnectChanged(newValue);
    }
}

bool AppSettings::alwaysSendToAll() const
{
    return d->alwaysSendToAll;
}

void AppSettings::setAlwaysSendToAll(bool alwaysSendToAll)
{
    if (alwaysSendToAll != d->alwaysSendToAll) {
        d->alwaysSendToAll = alwaysSendToAll;
        QSettings settings;
        settings.setValue("alwaysSendToAll", d->alwaysSendToAll);
        Q_EMIT alwaysSendToAllChanged(alwaysSendToAll);
    }
}

QStringList AppSettings::idleCategories() const
{
    return d->idleCategories;
}

void AppSettings::setIdleCategories(QStringList newCategories)
{
    qDebug() << Q_FUNC_INFO << newCategories;
    if(newCategories != d->idleCategories) {
        d->idleCategories = newCategories;
        QSettings settings;
        settings.setValue("idleCategories", d->idleCategories);
        Q_EMIT idleCategoriesChanged(newCategories);
    }
}

void AppSettings::addIdleCategory(const QString& category)
{
    if(!d->idleCategories.contains(category)) {
        d->idleCategories << category;
        QSettings settings;
        settings.setValue("idleCategories", d->idleCategories);
        Q_EMIT idleCategoriesChanged(d->idleCategories);
    }
}

void AppSettings::removeIdleCategory(const QString& category)
{
    if(d->idleCategories.contains(category)) {
        d->idleCategories.removeAll(category);
        QSettings settings;
        settings.setValue("idleCategories", d->idleCategories);
        Q_EMIT idleCategoriesChanged(d->idleCategories);
    }
}

int AppSettings::idleMinPause() const
{
    return d->idleMinPause;
}

void AppSettings::setIdleMinPause(int pause)
{
    qDebug() << Q_FUNC_INFO << pause;
    if(pause != d->idleMinPause) {
        d->idleMinPause = pause;
        QSettings settings;
        settings.setValue("idleMinPause", d->idleMinPause);
        Q_EMIT idleMinPauseChanged(pause);
    }
}

int AppSettings::idleMaxPause() const
{
    return d->idleMaxPause;
}

void AppSettings::setIdleMaxPause(int pause)
{
    qDebug() << Q_FUNC_INFO << pause;
    if(pause != d->idleMaxPause) {
        d->idleMaxPause = pause;
        QSettings settings;
        settings.setValue("idleMaxPause", d->idleMaxPause);
        Q_EMIT idleMaxPauseChanged(pause);
    }
}

bool AppSettings::fakeTailMode() const
{
    return d->fakeTailMode;
}

void AppSettings::setFakeTailMode(bool fakeTailMode)
{
    qDebug() << Q_FUNC_INFO << fakeTailMode;
    if(fakeTailMode != d->fakeTailMode) {
        d->fakeTailMode = fakeTailMode;
        QSettings settings;
        settings.setValue("fakeTailMode", d->fakeTailMode);
        Q_EMIT fakeTailModeChanged(fakeTailMode);
    }
}

QStringList AppSettings::moveLists() const
{
    QStringList keys = d->moveLists.keys();
    // For some decidedly silly reason, we end up going from no entries
    // to two entries when adding the first one, and we then have a ghost one
    if(keys.count() > 0 && keys.at(0).isEmpty()) {
        keys.takeAt(0);
    }
    return keys;
}

void AppSettings::addMoveList(const QString& moveListName)
{
    if(!moveListName.isEmpty()) {
        d->moveLists.insert(moveListName, QStringList());
        Q_EMIT moveListsChanged(moveLists());
    }
}

void AppSettings::removeMoveList(const QString& moveListName)
{
    d->moveLists.remove(moveListName);
    Q_EMIT moveListsChanged(d->moveLists.keys());
}

QStringList AppSettings::moveList() const
{
    return d->moveLists[d->activeMoveListName];
}

void AppSettings::setActiveMoveList(const QString& moveListName)
{
    d->activeMoveListName = moveListName;
    Q_EMIT moveListChanged(moveList());
}

void AppSettings::addMoveListEntry(int index, const QString& entry, QStringList devices)
{
    QStringList moveList = d->moveLists[d->activeMoveListName];
    moveList.insert(index, entry);
    d->moveLists[d->activeMoveListName] = moveList;
    Q_EMIT moveListChanged(this->moveList());
}

void AppSettings::removeMoveListEntry(int index)
{
    QStringList moveList = d->moveLists[d->activeMoveListName];
    moveList.removeAt(index);
    d->moveLists[d->activeMoveListName] = moveList;
    Q_EMIT moveListChanged(this->moveList());
}

AlarmList * AppSettings::alarmListImpl() const
{
    return d->alarmList;
}

QVariantList AppSettings::alarmList() const
{
    return d->alarmList->toVariantList();
}

void AppSettings::addAlarm(const QString& alarmName)
{
    d->alarmList->addAlarm(alarmName);
}

void AppSettings::setDeviceName(const QString& address, const QString& deviceName)
{
    QSettings settings;
    settings.beginGroup("DeviceNameList");
    settings.setValue(address, deviceName);
    settings.endGroup();
    Q_EMIT deviceNamesChanged(deviceNames());
}

void AppSettings::clearDeviceNames()
{
    QSettings settings;
    settings.remove("DeviceNameList");
    Q_EMIT deviceNamesChanged(deviceNames());
}

QVariantMap AppSettings::deviceNames() const
{
    QVariantMap namesMap;
    QSettings settings;
    settings.beginGroup("DeviceNameList");
    QStringList keys = settings.childKeys();
    for(const QString &key : std::as_const(keys)) {
        namesMap[key] = settings.value(key).toString();
    }
    settings.endGroup();
    return namesMap;
}

QStringList AppSettings::availableLanguages() const
{
    static QStringList languages;
    if (languages.isEmpty()) {
        QSet<QString> codes = KLocalizedString::availableApplicationTranslations();
        for (const QString& code : codes) {
            QLocale locale(code);
            languages << QString::fromUtf8("%1 (%2)").arg(locale.nativeLanguageName()).arg(code);
        }
        languages.sort();
        languages.prepend(QLatin1String{"System Language"});
    }
    return languages;
}

QString AppSettings::languageOverride() const
{
    return d->languageOverride;
}

static const QLatin1String paranthesisStart{")"};
static const QLatin1String paranthesisEnd{")"};
void AppSettings::setLanguageOverride(QString languageOverride)
{
    if (d->languageOverride != languageOverride) {
        QString languageCode = languageOverride;
        if (languageOverride.endsWith(paranthesisEnd)) {
            int firstPos = languageCode.lastIndexOf(paranthesisStart) + 1;
            int lastPos = languageCode.lastIndexOf(paranthesisEnd);
            languageCode = languageCode.mid(firstPos, lastPos - firstPos);
        }
        d->languageOverride = languageCode;
        QSettings settings;
        settings.setValue("languageOverride", d->languageOverride);
        Q_EMIT languageOverrideChanged(languageOverride);
        if (d->languageOverride.isEmpty()) {
            KLocalizedString::clearLanguages();
        } else {
            KLocalizedString::setLanguages(QStringList() << d->languageOverride << QLatin1String{"en_US"});
        }
    }
}

void AppSettings::removeAlarm(const QString& alarmName)
{
    d->alarmList->removeAlarm(alarmName);
}

QVariantMap AppSettings::activeAlarm() const
{
    return d->alarmList->getAlarmVariantMap(d->activeAlarmName);
}

void AppSettings::setActiveAlarmName(const QString &alarmName)
{
    if (d->activeAlarmName != alarmName) {
        d->activeAlarmName = alarmName;
        Q_EMIT activeAlarmChanged(activeAlarm());
    }
}

void AppSettings::changeAlarmName(const QString &newName)
{
    d->alarmList->changeAlarmName(d->activeAlarmName, newName);
    Q_EMIT activeAlarmChanged(activeAlarm());
}

void AppSettings::setAlarmTime(const QDateTime &time)
{
    d->alarmList->setAlarmTime(d->activeAlarmName, time);
    Q_EMIT activeAlarmChanged(activeAlarm());
}

void AppSettings::setAlarmCommands(const QStringList& commands)
{
    d->alarmList->setAlarmCommands(d->activeAlarmName, commands);
    Q_EMIT activeAlarmChanged(activeAlarm());
}

void AppSettings::addAlarmCommand(int index, const QString& command, QStringList devices)
{
    d->alarmList->addAlarmCommand(d->activeAlarmName, index, command, devices);
    Q_EMIT activeAlarmChanged(activeAlarm());
}

void AppSettings::removeAlarmCommand(int index)
{
    d->alarmList->removeAlarmCommand(d->activeAlarmName, index);
    Q_EMIT activeAlarmChanged(activeAlarm());
}

void AppSettings::loadAlarmList()
{
    QSettings settings;

    settings.beginGroup("AlarmList");
    int size = settings.beginReadArray("Alarms");

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        const QString name = settings.value("name").toString();
        const QDateTime time = settings.value("time").toDateTime();
        const QStringList commands = settings.value("commands").toStringList();

        d->alarmList->addAlarm(name, time, commands);
    }

    settings.endArray();
    settings.endGroup();
}

void AppSettings::saveAlarmList()
{
    QSettings settings;

    settings.beginGroup("AlarmList");
    settings.beginWriteArray("Alarms", d->alarmList->size());

    for (int i = 0; i < d->alarmList->size(); ++i) {
        Alarm *alarm = d->alarmList->at(i);
        settings.setArrayIndex(i);

        settings.setValue("name", alarm->name());
        settings.setValue("time", alarm->time());
        settings.setValue("commands", alarm->commands());
    }

    settings.endArray();
    settings.endGroup();
}

void AppSettings::onAlarmListChanged()
{
    saveAlarmList();
    Q_EMIT alarmListChanged(alarmList());
}

void AppSettings::shutDownService()
{
    qApp->quit();
}

QVariantMap AppSettings::commandFiles() const
{
    return d->commandFiles;
}

static const QLatin1String emptyString{""};
void AppSettings::addCommandFile(const QString& filename, const QString& content)
{
    QVariantMap fileMap;
    fileMap[QLatin1String{"contents"}] = emptyString;
    fileMap[QLatin1String{"title"}] = emptyString;
    fileMap[QLatin1String{"description"}] = emptyString;
    fileMap[QLatin1String{"isEditable"}] = true;
    fileMap[QLatin1String{"isValid"}] = false;
    d->commandFiles[filename] = fileMap;
    setCommandFileContents(filename, content);
}

void AppSettings::removeCommandFile(const QString& filename)
{
    QVariantMap fileMap = d->commandFiles[filename].toMap();
    if (fileMap[QLatin1String{"isEditable"}].toBool()) {
        d->commandFiles.remove(filename);
        QFile theFile{filename};
        if (theFile.exists()) {
            theFile.remove();
        }
        Q_EMIT commandFilesChanged(d->commandFiles);
    }
}

void AppSettings::setCommandFileContents(const QString& filename, const QString& content)
{
// [QString (Title)] => QString - A short title for the file as interpreted from the contents on load
// [QString (Description)] => QString - The long-form description of the file as interpreted from the contents on load
// [QString (Contents)] => QString - The actual contents of the file
// [QString (Editable)] => bool - Whether or not the contents can be changed (false when the file is a built-in)
// [QString (Valid)] => bool - Whether or not the contents are valid json/crumpet

    QVariantMap fileMap = d->commandFiles[filename].toMap();
    if (fileMap[QLatin1String{"isEditable"}].toBool()) {
        // Don't store the things back if we're not yet initialised, or we'll just be writing stuff we
        // just read, which seems silly...
        if (d->isInitialized) {
            // Store into the settings instance - we will want to make this file editing later
            // but for now it allows us to not ask for file access permissions
            QSettings settings;
            settings.beginGroup("CrumpetFiles");
            settings.setValue(filename, content);
            settings.endGroup();
            settings.sync();
        }

        fileMap[QLatin1String{"contents"}] = content;
        fileMap[QLatin1String{"isValid"}] = false;

        CommandPersistence persistence;
        persistence.deserialize(content);
        if (persistence.error().isEmpty()) {
            fileMap[QLatin1String{"title"}] = persistence.title();
            fileMap[QLatin1String{"description"}] = persistence.description();
            fileMap[QLatin1String{"isValid"}] = true;
        }
        d->commandFiles[filename] = fileMap;
        Q_EMIT commandFilesChanged(d->commandFiles);
    }
}

void AppSettings::renameCommandFile(const QString& filename, const QString& newFilename)
{
    QVariantMap fileMap = d->commandFiles.take(filename).toMap();
    if (fileMap[QLatin1String{"isEditable"}].toBool()) {
        d->commandFiles[newFilename] = fileMap;
        Q_EMIT commandFilesChanged(d->commandFiles);
    }
}
