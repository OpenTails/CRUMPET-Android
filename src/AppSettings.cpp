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
#include "PhoneEventList.h"
#include "PhoneEvent.h"

#include <QSettings>

class AppSettings::Private
{
public:
    Private()
    {
        idleCategories << "relaxed";
    }
    ~Private() {}

    bool advancedMode = false;
    bool developerMode = false;
    bool idleMode = false;
    bool autoReconnect = true;
    QStringList idleCategories;
    int idleMinPause = 15;
    int idleMaxPause = 60;

    QMap<QString, QStringList> moveLists;
    QString activeMoveListName;

    AlarmList* alarmList = nullptr;
    QString activeAlarmName;

    PhoneEventList* phoneEventList = nullptr;
    QString activePhoneEventName;
};

AppSettings::AppSettings(QObject* parent)
    : SettingsProxySource(parent)
    , d(new Private)
{
    QSettings settings;

    d->advancedMode = settings.value("advancedMode", d->advancedMode).toBool();
    d->developerMode = settings.value("developerMode", d->developerMode).toBool();
    d->idleMode = settings.value("idleMode", d->idleMode).toBool();
    d->autoReconnect = settings.value("autoReconnect", d->autoReconnect).toBool();
    d->idleCategories = settings.value("idleCategories", d->idleCategories).toStringList();
    d->idleMinPause = settings.value("idleMinPause", d->idleMinPause).toInt();
    d->idleMaxPause = settings.value("idleMaxPause", d->idleMaxPause).toInt();

    settings.beginGroup("MoveLists");
    QStringList moveLists = settings.allKeys();
    for(const QString& list : moveLists) {
        d->moveLists[list] = settings.value(list).toString().split(';');
    }
    settings.endGroup();

    auto saveMoveLists = [this](){
        QSettings settings;
        settings.beginGroup("MoveLists");
        QMap<QString, QStringList>::const_iterator i = d->moveLists.constBegin();
        while(i != d->moveLists.constEnd()) {
            if(!i.key().isEmpty()) {
                settings.setValue(i.key(), i.value().join(';'));
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

    d->phoneEventList = new PhoneEventList(this);

    loadPhoneEventList();

    connect(d->phoneEventList, &PhoneEventList::listChanged, this, &AppSettings::onPhoneEventListChanged);
}

AppSettings::~AppSettings()
{
    delete d;
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
        emit advancedModeChanged(newValue);
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
        emit developerModeChanged(newValue);
    }
}

bool AppSettings::idleMode() const
{
    return d->idleMode;
}

void AppSettings::setIdleMode(bool newValue)
{
    qDebug() << Q_FUNC_INFO << newValue;
    if(newValue != d->idleMode) {
        d->idleMode = newValue;
        QSettings settings;
        settings.setValue("idleMode", d->idleMode);
        emit idleModeChanged(newValue);
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
        emit autoReconnectChanged(newValue);
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
        emit idleCategoriesChanged(newCategories);
    }
}

void AppSettings::addIdleCategory(const QString& category)
{
    if(!d->idleCategories.contains(category)) {
        d->idleCategories << category;
        QSettings settings;
        settings.setValue("idleCategories", d->idleCategories);
        emit idleCategoriesChanged(d->idleCategories);
    }
}

void AppSettings::removeIdleCategory(const QString& category)
{
    if(d->idleCategories.contains(category)) {
        d->idleCategories.removeAll(category);
        QSettings settings;
        settings.setValue("idleCategories", d->idleCategories);
        emit idleCategoriesChanged(d->idleCategories);
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
        emit idleMinPauseChanged(pause);
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
        emit idleMaxPauseChanged(pause);
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
        emit moveListsChanged(moveLists());
    }
}

void AppSettings::removeMoveList(const QString& moveListName)
{
    d->moveLists.remove(moveListName);
    emit moveListsChanged(d->moveLists.keys());
}

QStringList AppSettings::moveList() const
{
    return d->moveLists[d->activeMoveListName];
}

void AppSettings::setActiveMoveList(const QString& moveListName)
{
    d->activeMoveListName = moveListName;
    emit moveListChanged(moveList());
}

void AppSettings::addMoveListEntry(int index, const QString& entry)
{
    QStringList moveList = d->moveLists[d->activeMoveListName];
    moveList.insert(index, entry);
    d->moveLists[d->activeMoveListName] = moveList;
    emit moveListChanged(this->moveList());
}

void AppSettings::removeMoveListEntry(int index)
{
    QStringList moveList = d->moveLists[d->activeMoveListName];
    moveList.removeAt(index);
    d->moveLists[d->activeMoveListName] = moveList;
    emit moveListChanged(this->moveList());
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
    emit deviceNamesChanged(deviceNames());
}

void AppSettings::clearDeviceNames()
{
    QSettings settings;
    settings.remove("DeviceNameList");
    emit deviceNamesChanged(deviceNames());
}

QVariantMap AppSettings::deviceNames() const
{
    QVariantMap namesMap;
    QSettings settings;
    settings.beginGroup("DeviceNameList");
    QStringList keys = settings.childKeys();
    foreach(QString key, keys) {
        namesMap[key] = settings.value(key).toString();
    }
    settings.endGroup();
    return namesMap;
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
        emit activeAlarmChanged(activeAlarm());
    }
}

void AppSettings::changeAlarmName(const QString &newName)
{
    d->alarmList->changeAlarmName(d->activeAlarmName, newName);
    emit activeAlarmChanged(activeAlarm());
}

void AppSettings::setAlarmTime(const QDateTime &time)
{
    d->alarmList->setAlarmTime(d->activeAlarmName, time);
    emit activeAlarmChanged(activeAlarm());
}

void AppSettings::setAlarmCommands(const QStringList& commands)
{
    d->alarmList->setAlarmCommands(d->activeAlarmName, commands);
    emit activeAlarmChanged(activeAlarm());
}

void AppSettings::addAlarmCommand(int index, const QString& command)
{
    d->alarmList->addAlarmCommand(d->activeAlarmName, index, command);
    emit activeAlarmChanged(activeAlarm());
}

void AppSettings::removeAlarmCommand(int index)
{
    d->alarmList->removeAlarmCommand(d->activeAlarmName, index);
    emit activeAlarmChanged(activeAlarm());
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

void AppSettings::loadPhoneEventList()
{
    QSettings settings;

    settings.beginGroup("PhoneEventList");
    int size = settings.beginReadArray("PhoneEvents");

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        const QString name = settings.value("name").toString();
        const QStringList commands = settings.value("commands").toStringList();

        d->phoneEventList->setPhoneEventCommands(name, commands);
    }

    settings.endArray();
    settings.endGroup();

}

void AppSettings::savePhoneEventList()
{
    QSettings settings;

    settings.beginGroup("PhoneEventList");
    settings.beginWriteArray("PhoneEvents", d->phoneEventList->size());

    for (int i = 0; i < d->phoneEventList->size(); ++i) {
        PhoneEvent *phoneEvent = d->phoneEventList->at(i);
        settings.setArrayIndex(i);

        settings.setValue("name", phoneEvent->name());
        settings.setValue("commands", phoneEvent->commands());
    }

    settings.endArray();
    settings.endGroup();
}

void AppSettings::onAlarmListChanged()
{
    saveAlarmList();
    emit alarmListChanged(alarmList());
}

void AppSettings::onPhoneEventListChanged()
{
    savePhoneEventList();
    emit phoneEventListChanged(phoneEventList());
}

PhoneEventList *AppSettings::phoneEventListImpl() const
{
    return d->phoneEventList;
}

QVariantList AppSettings::phoneEventList() const
{
    return d->phoneEventList->toVariantList();
}

QVariantMap AppSettings::activePhoneEvent() const
{
    return d->phoneEventList->getPhoneEventVariantMap(d->activePhoneEventName);
}

void AppSettings::handlePhoneEvent(const QString& callType)
{
    phoneEventListImpl()->handle(callType);
}

void AppSettings::setActivePhoneEventName(const QString &phoneEventName)
{
    if (d->activePhoneEventName != phoneEventName) {
        d->activePhoneEventName = phoneEventName;
        emit activePhoneEventChanged(activePhoneEvent());
    }
}

void AppSettings::setPhoneEventCommands(const QStringList &commands)
{
    d->phoneEventList->setPhoneEventCommands(d->activePhoneEventName, commands);
    emit activePhoneEventChanged(activePhoneEvent());
}

void AppSettings::addPhoneEventCommand(int index, const QString &command)
{
    d->phoneEventList->addPhoneEventCommand(d->activePhoneEventName, index, command);
    emit activePhoneEventChanged(activePhoneEvent());
}

void AppSettings::removePhoneEventCommand(int index)
{
    d->phoneEventList->removePhoneEventCommand(d->activePhoneEventName, index);
    emit activePhoneEventChanged(activePhoneEvent());
}
