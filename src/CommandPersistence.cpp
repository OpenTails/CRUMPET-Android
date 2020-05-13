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

#include <QDir>
#include <QStandardPaths>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class CommandPersistence::Private {
public:
    Private(CommandPersistence* q)
        : q(q)
    {}
    ~Private() {}
    CommandPersistence* q;

    QString filename;
    CommandInfoList commands;
    QString title;
    QString description;

    void reportError(const QString& message) {
        error = message;
        emit q->error(message);
    }
    QString error;

    QString pathName() {
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QDir directory{path};
        if (!directory.exists()) {
            if (!directory.mkpath(QLatin1String{"."})) {
                reportError(QLatin1String{"Failed to create the directory for the stored commands"});
                return QString{};
            }
        }
        return QString{"%1/commands/%2.crumpet"}.arg(path).arg(filename);
    }
};

CommandPersistence::CommandPersistence(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
}

CommandPersistence::~CommandPersistence()
{
    delete d;
}

bool CommandPersistence::read()
{
    bool keepgoing{true};
    QString pathName = d->pathName();
    keepgoing = pathName.isEmpty();
    if (keepgoing) {
        QFile file(pathName);
        keepgoing = file.open(QIODevice::ReadOnly);
        if (keepgoing) {
            QByteArray data = file.readAll();
            file.close();

            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
            if ((keepgoing = (parseError.error != QJsonParseError::NoError))) {
                QJsonObject obj{doc.object()};
                // These two being optional means we just ignore it if they're empty
                setTitle(obj.value(QLatin1String{"Title"}).toString());
                setDescription(obj.value(QLatin1String{"Description"}).toString());
                QJsonArray commands{obj.value(QLatin1String{"Commands"}).toArray()};
                CommandInfoList commandsList;
                if ((keepgoing = (commands.count() > 0))) {
                    for (const QJsonValue& val : qAsConst(commands)) {
                        QJsonObject commandObject{val.toObject()};
                        CommandInfo info;
                        info.name = commandObject.value(QLatin1String{"Name"}).toString();
                        info.command = commandObject.value(QLatin1String{"Command"}).toString();
                        info.category = commandObject.value(QLatin1String{"Category"}).toString();
                        info.duration = commandObject.value(QLatin1String{"Duration"}).toInt(); // milliseconds
                        info.minimumCooldown = commandObject.value(QLatin1String{"MinimumCooldown"}).toInt(); // milliseconds
                        info.group = commandObject.value(QLatin1String{"Group"}).toInt();
                        commandsList.append(info);
                    }
                }
                else {
                    d->reportError(QString{"There are no commands in this file. This is possible, but not common."});
                }
                // The commands list still gets cleared out when loading a file with no commands in it
                setCommands(commandsList);
            }
            else {
                d->reportError(QString{"Failed to load your commands, due to a parsing error at %1: %2"}.arg(parseError.offset).arg(parseError.errorString()));
            }
        }
        else {
            d->reportError(QString{"Could not open the file %1 for reading"}.arg(pathName));
        }
    }
    // No need to report error, the pathname generator already did that
    return keepgoing;
}

bool CommandPersistence::write()
{
    return false;
}

QString CommandPersistence::error() const
{
    return d->error;
}

QString CommandPersistence::filename() const
{
    return d->filename;
}

void CommandPersistence::setFilename(const QString& filename)
{
    if (d->filename != filename) {
        d->filename = filename;
        emit filenameChanged();
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
