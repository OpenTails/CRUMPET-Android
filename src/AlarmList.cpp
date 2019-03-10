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

#include "CommandQueue.h"

#include <QDebug>
#include <QTimer>

class AlarmList::Private
{
public:
    Private(AlarmList* qq)
        : q(qq)
        , alarmTimer(new QTimer(qq))
    {
        // We run to the minute, so wake up every half minute to check if we have an alarm coming up
        // NB: The timer only runs when there is more than zero alarms in the list.
        alarmTimer->setInterval(0.5 * 60000);
        QObject::connect(alarmTimer, &QTimer::timeout, qq, [this](){ checkAlarms(); });
    }
    AlarmList* q;
    CommandQueue* commandQueue = nullptr;

    QList<Alarm*> list;

    QTimer* alarmTimer = nullptr;
    void checkAlarms() {
        if(!commandQueue) {
            qDebug() << "You forgot to set the command queue on the alarm list, silly person!";
            return;
        }
        quint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
        quint64 interval(alarmTimer->interval());
        for( Alarm* alarm : list)
        {
            quint64 then = alarm->time().toMSecsSinceEpoch();
            if(now < then)
            {
//                 qDebug() << "Event is in the future, let's see if it's near enough...";
                quint64 until = then - now;
                if(until < interval)
                {
//                     qDebug() << "Event is within our check interval, so launch it now";
                    commandQueue->clear();
                    commandQueue->pushPause(until);
                    commandQueue->pushCommands(alarm->commands());
                    break;
                }
                // TODO This doesn't handle two alarms set to go off within the same interval (in other words
                // set on the same minute point). If this turns out to be something people do, we can fix that,
                // but until then, let's not worry about that...
            }
        }
    }
};

AlarmList::AlarmList(QObject* parent)
    : QAbstractListModel(parent),
      d(new Private(this))
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

    // Once we've added an alarm, start the timer
    d->alarmTimer->start();
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

    // When there are no more alarms, stop the timer
    if(d->list.count() == 0) {
        d->alarmTimer->stop();
    }
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

void AlarmList::setCommandQueue(CommandQueue* commandQueue)
{
    d->commandQueue = commandQueue;
}
