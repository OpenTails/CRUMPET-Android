/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "CommandPersistence.h"

class CommandPersistence::Private {
public:
    Private() {}
    ~Private() {}

    QUrl location;
    CommandInfoList commands;
    QString title;
    QString description;

    QString error;
};

CommandPersistence::CommandPersistence(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

CommandPersistence::~CommandPersistence()
{
    delete d;
}

bool CommandPersistence::read()
{
    return false;
}

bool CommandPersistence::write()
{
    return false;
}

QString CommandPersistence::error() const
{
    return d->error;
}

QUrl CommandPersistence::location() const
{
    return d->location;
}

void CommandPersistence::setLocation(const QUrl& location)
{
    if (d->location != location) {
        d->location = location;
        emit locationChanged();
    }
}

CommandInfoList CommandPersistence::commands() const
{
    return d->commands;
}

void CommandPersistence::setCommands(const CommandInfoList& commands)
{
    d->commands = commands;
    emit commandsChanged();
}

QString CommandPersistence::title() const
{
    return d->title;
}

void CommandPersistence::setTitle(const QString& title)
{
    if (d->title != title) {
        d->title = title;
        emit titleChanged();
    }
}

QString CommandPersistence::description() const
{
    return d->description;
}

void CommandPersistence::setDescription(const QString& description)
{
    if (d->description != description) {
        d->description = description;
        emit descriptionChanged();
    }
}
