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

#include "Alarm.h"

#include <QDebug>

class Alarm::Private
{
public:
    Private()
    {}

    Private(const QString& name)
        : name(name),
          time(QDateTime::currentDateTime())
    {}

    Private(const QString& name, const QDateTime& time, const QStringList& commands)
        : name(name),
          time(time),
          commands(commands)
    {
        if (!this->time.isValid()) {
            this->time = QDateTime::currentDateTime();
        }
    }

    QString name;
    QDateTime time;

    //TODO: It would be great to represent commands as objects, not strings
    QStringList commands;
};

Alarm::Alarm(QObject* parent)
    : QObject(parent),
      d(new Private())
{
}

Alarm::Alarm(const QString& name, QObject* parent)
    : QObject(parent),
      d(new Private(name))
{
}

Alarm::Alarm(const QString& name,
             const QDateTime& time,
             const QStringList& commands,
             QObject* parent)
    : QObject(parent),
      d(new Private(name, time, commands))
{
}

Alarm::~Alarm()
{
    delete d;
}

QString Alarm::name() const
{
    return d->name;
}

void Alarm::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        Q_EMIT nameChanged();
        Q_EMIT alarmChanged();
    }
}

QDateTime Alarm::time() const
{
    return d->time;
}

void Alarm::setTime(const QDateTime& time)
{
    if (d->time != time) {
        d->time = time;
        Q_EMIT timeChanged();
        Q_EMIT alarmChanged();
    }
}

QStringList Alarm::commands() const
{
    return d->commands;
}

void Alarm::setCommands(const QStringList& commands)
{
    d->commands = commands;
    Q_EMIT commandsChanged();
    Q_EMIT alarmChanged();
}

void Alarm::addCommand(int index, const QString& command)
{
    d->commands.insert(index, command);
    Q_EMIT commandsChanged();
    Q_EMIT alarmChanged();
}

void Alarm::removeCommand(int index)
{
    if (index < 0 || index >= d->commands.size()) {
        qWarning() << QString::fromUtf8("Unable to remvoe command from alarm '%1'. Index (%2) is out of the bounds.")
                      .arg(name())
                      .arg(index);
        return;
    }

    d->commands.removeAt(index);
    Q_EMIT commandsChanged();
    Q_EMIT alarmChanged();
}


QVariantMap Alarm::toVariantMap() const
{
    QVariantMap result;

    result[QLatin1String{"name"}] = name();
    result[QLatin1String{"time"}] = time();
    result[QLatin1String{"commands"}] = commands();

    return result;
}
