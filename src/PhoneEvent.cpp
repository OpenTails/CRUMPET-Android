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

#include "PhoneEvent.h"

#include <QDebug>

class PhoneEvent::Private
{
public:
    Private(PhoneEventList::Event event)
        : event(event)
    {}

    Private(PhoneEventList::Event event, const QStringList& commands)
        : event(event),
          commands(commands)
    {
    }

    const PhoneEventList::Event event;

    //TODO: It would be great to represent commands as objects, not strings
    QStringList commands;
};

PhoneEvent::PhoneEvent(PhoneEventList::Event event, QObject* parent)
    : QObject(parent),
      d(new Private(event))
{
}

PhoneEvent::PhoneEvent(PhoneEventList::Event event,
                       const QStringList& commands,
                       QObject* parent)
    : QObject(parent),
      d(new Private(event, commands))
{
}

PhoneEvent::~PhoneEvent()
{
    delete d;
}

PhoneEventList::Event PhoneEvent::phoneEvent() const
{
    return d->event;
}

QString PhoneEvent::name() const
{
    return PhoneEventList::getEventName(d->event);
}

QString PhoneEvent::displayName() const
{
    return PhoneEventList::getEventDisplayName(d->event);
}

bool PhoneEvent::isEmpty() const
{
    return d->commands.isEmpty();
}

QStringList PhoneEvent::commands() const
{
    return d->commands;
}

void PhoneEvent::setCommands(const QStringList& commands)
{
    d->commands = commands;
    emit commandsChanged();
    emit phoneEventChanged();
}

void PhoneEvent::addCommand(int index, const QString& command)
{
    d->commands.insert(index, command);
    emit commandsChanged();
    emit phoneEventChanged();
}

void PhoneEvent::removeCommand(int index)
{
    if (index < 0 || index >= d->commands.size()) {
        qWarning() << QString("Unable to remvoe command from phone event '%1'. Index (%2) is out of the bounds.")
                      .arg(name())
                      .arg(index);
        return;
    }

    d->commands.removeAt(index);
    emit commandsChanged();
    emit phoneEventChanged();
}

QVariantMap PhoneEvent::toVariantMap() const
{
    QVariantMap result;

    result["name"] = name();
    result["displayName"] = displayName();
    result["commands"] = commands();

    return result;
}
