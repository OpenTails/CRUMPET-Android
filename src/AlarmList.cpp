/*
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

#include "AlarmList.h"
#include "Alarm.h"

#include <QDebug>

class AlarmList::Private
{
public:
    QList<Alarm*> list;
    QHash<int, QByteArray> roles;
};

AlarmList::AlarmList(QObject* parent)
    : QAbstractListModel(parent),
      d(new Private())
{
}

AlarmList::~AlarmList()
{
    delete d;
}

int AlarmList::size() const
{
    return d->list.size();
}

Alarm* AlarmList::at(int index) const
{
    if(index >= 0 && index < d->list.count()) {
        return d->list.at(index);
    };

    return nullptr;
}

QHash<int, QByteArray> AlarmList::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[AlarmRole] = "alarm";

    return roles;
}

int AlarmList::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->list.size();
}

QVariant AlarmList::data(const QModelIndex& index, int role) const
{
    if(index.isValid() && index.row() >= 0 && index.row() < d->list.count()) {
        if (role == AlarmRole) {
            return QVariant::fromValue(d->list.at(index.row()));
        }
    };

    return QVariant();
}

bool AlarmList::exists(const QString& name) const
{
    return alarm(name);
}

Alarm* AlarmList::alarm(const QString& name) const
{
    for (Alarm* alarm : d->list) {
        if (alarm->name() == name) {
            return alarm;
        }
    }

    return nullptr;
}

int AlarmList::alarmIndex(const QString& name) const
{
    for (int i = 0; i < d->list.size(); ++i) {
        if (d->list.at(i)->name() == name) {
            return i;
        }
    }

    return -1;;
}

void AlarmList::addAlarm(const QString& name, const QDateTime& time, const QStringList& commands)
{
    Alarm* alarm = new Alarm(name, time, commands, this);
    addAlarm(alarm);
}

void AlarmList::addAlarm(Alarm* alarm)
{
    if (d->list.contains(alarm)) {
        qWarning() << "The alarm already exists in Alarm List";
        emit alarmExisted(alarm->name());
        return;
    }

    if (exists(alarm->name())) {
        emit alarmExisted(alarm->name());
        return;
    }

    beginInsertRows(QModelIndex(), 0, 0);
    alarm->setParent(this);

    connect(alarm, &Alarm::alarmChanged, this, &AlarmList::listChanged);

    d->list.insert(0, alarm);
    emit listChanged();
    endInsertRows();
}

void AlarmList::addAlarm(const QString& alarmName)
{
    Alarm* alarm = new Alarm(alarmName, this);
    addAlarm(alarm);
}

void AlarmList::removeAlarm(Alarm* alarm)
{
    int index = d->list.indexOf(alarm);

    if (index < 0) {
        qWarning() << "Unable to find the alarm in Alarm List";
        return;
    }

    removeAlarmByIndex(index);
}

void AlarmList::removeAlarm(const QString& alarmName)
{
    removeAlarmByIndex(alarmIndex(alarmName));
}

void AlarmList::removeAlarmByIndex(int index)
{
    if (index < 0 || index >= d->list.size()) {
        qWarning() << QString("Unable to delete alarm with index %1").arg(index);
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);

    Alarm* alarm = d->list.at(index);
    d->list.removeAt(index);
    disconnect(alarm);
    alarm->deleteLater();

    emit listChanged();
    endRemoveRows();
}

void AlarmList::changeAlarmName(const QString& oldName, const QString& newName)
{
    if (oldName == newName) {
        return;
    }

    if (exists(newName)) {
        emit alarmExisted(newName);
        return;
    }

    Alarm* alarm = this->alarm(oldName);

    if (alarm) {
        alarm->setName(newName);
    } else {
        emit alarmNotExisted(oldName);
    }
}

void AlarmList::setAlarmTime(const QString& alarmName, const QDateTime& time)
{
    Alarm* alarm = this->alarm(alarmName);

    if (alarm) {
        alarm->setTime(time);
    } else {
        emit alarmNotExisted(alarmName);
    }
}

void AlarmList::setAlarmCommands(const QString& alarmName, const QStringList& commands)
{
    Alarm* alarm = this->alarm(alarmName);

    if (alarm) {
        alarm->setCommands(commands);
    } else {
        emit alarmNotExisted(alarmName);
    }
}

void AlarmList::addAlarmCommand(const QString& alarmName, int index, const QString& command)
{
    Alarm* alarm = this->alarm(alarmName);

    if (alarm) {
        alarm->addCommand(index, command);
    } else {
        emit alarmNotExisted(alarmName);
    }
}

void AlarmList::removeAlarmCommand(const QString& alarmName, int index)
{
    Alarm* alarm = this->alarm(alarmName);

    if (alarm) {
        alarm->removeCommand(index);
    } else {
        emit alarmNotExisted(alarmName);
    }
}

QVariantList AlarmList::toVariantList() const
{
    QVariantList result;

    for (const Alarm* alarm : d->list) {
        result.push_back(alarm->toVariantMap());
    }

    return result;
}

QVariantMap AlarmList::getAlarmVariantMap(const QString& alarmName)
{
    if (alarmName.isEmpty()) {
        return QVariantMap();
    }

    Alarm* alarm = this->alarm(alarmName);

    if (alarm) {
        return alarm->toVariantMap();
    } else {
        emit alarmNotExisted(alarmName);
        return QVariantMap();
    }
}
