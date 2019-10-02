/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "TailCommandModel.h"

#include <QDebug>
#include <QRandomGenerator>

class TailCommandModel::Private
{
public:
    Private() {}
    ~Private() {}

    QList<TailCommandModel::CommandInfo*> commands;
    QString tailVersion;
};

TailCommandModel::TailCommandModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{}

TailCommandModel::~TailCommandModel()
{
    delete d;
}

QHash< int, QByteArray > TailCommandModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {Name, "name"},
        {Command, "command"},
        {IsRunning, "isRunning"},
        {Category, "category"},
        {Duration, "duration"},
        {MinimumCooldown, "minimumCooldown"},
        {CommandIndex, "commandIndex"}};
    return roles;
}

QVariant TailCommandModel::data(const QModelIndex& index, int role) const
{
    QVariant value;
    if(index.isValid() && index.row() > -1 && index.row() < d->commands.count()) {
        CommandInfo* command = d->commands.at(index.row());
        switch(role) {
            case Name:
                value = command->name;
                break;
            case Command:
                value = command->command;
                break;
            case IsRunning:
                value = command->isRunning;
                break;
            case Category:
                value = command->category;
                break;
            case Duration:
                value = command->duration;
                break;
            case MinimumCooldown:
                value = command->minimumCooldown;
                break;
            case CommandIndex:
                value = index.row();
                break;
            default:
                break;
        }
    };
    return value;
}

int TailCommandModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return d->commands.count();
}

void TailCommandModel::clear()
{
    beginResetModel();
    qDeleteAll(d->commands);
    d->commands.clear();
    endResetModel();
}

void TailCommandModel::addCommand(CommandInfo* command)
{
    beginInsertRows(QModelIndex(), 0, 0);
    d->commands.insert(0, command);
    endInsertRows();
}

void TailCommandModel::removeCommand(CommandInfo* command)
{
    int idx = d->commands.indexOf(command);
    if (idx > -1) {
        beginRemoveRows(QModelIndex(), idx, idx);
        d->commands.removeAt(idx);
        endRemoveRows();
    }
    delete command;
}

void TailCommandModel::setRunning(const QString& command, bool isRunning)
{
//     qDebug() << "Command changing running state" << command << "being set to" << isRunning;
    int i = -1;
    foreach(TailCommandModel::CommandInfo* theCommand, d->commands) {
        ++i;
//         qDebug() << "Checking" << theCommand->command << "named" << theCommand->name;
        if(theCommand->command == command) {
//             qDebug() << "Found matching command!" << i;
            if(theCommand->isRunning != isRunning) {
//                 qDebug() << "Changing state";
                QModelIndex idx = index(i, 0);
                theCommand->isRunning = isRunning;
                dataChanged(idx, idx);
            }
            break;
        }
    }
//     qDebug() << "Done changing command running state";
}

void TailCommandModel::autofill(const QString& version)
{
// ### TAIL MOVES AND DURATION
// ```
// TAILS1 - SLOW_WAG1 11.53 secs
// TAILS2 - SLOW_WAG2 7.10 secs
// TAILS3 - SLOW_WAG3 8.50 secs
// TAILFA - FAST_WAG 9.96 secs
// TAILSH - SHORT_WAG 7.46 secs
// TAILHA - HAPPY_WAG 8.86 secs
// TAILER - ERECT 5.80 secs
// TAILEP - ERECT_PULSE 9.76 secs
// TAILT1 - TREMBLE1 4.06 secs
// TAILT2 - TREMBLE2 5.55 secs
// TAILET - ERECT_TREM 4.73 secs
// ```
// and a Home Move: TAILHM
//
// There are 6 built in LED patterns
//
// ### LED PATTERNS
// ```
// LEDREC - intermittent (on/off)
// LEDTRI - triangular (fade in/out)
// LEDSAW - sawtooth (fade in/off)
// LEDSOS - morse SOS
// LEDFLA - flame simulation
// LEDSTR - strobo
// ```
// and the LEDOFF command
// ### OTHER COMMANDS
// ```
// VER - returns the firmware version number
// PING - returns OK
// SHUTDOWN - powers off the unit (will lose the connection!)
// BATT - returns Bn (n= 0,1,2,3,4 number of 'bars')
// USERMOVE and USERLEDS which deal with user created moves or LED patterns
// ```

    beginResetModel();
    qDeleteAll(d->commands);
    d->commands.clear();
    d->tailVersion = version;
    if (version == QLatin1String()) {
        // Herpetyderp, no version, what's this?
    }
    else /* if (version == QLatin1String("the version for this tail version...")) */ {
        qDebug() << "Adding commands for tail version" << version;
        CommandInfo* command = new CommandInfo();
        command->name = QLatin1String("Tail Home Position");
        command->command = QLatin1String("TAILHM");
        command->category = QLatin1String("");
        command->duration = 200;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Slow Wag 1");
        command->command = QLatin1String("TAILS1");
        command->category = QLatin1String("relaxed");
        command->duration = 11530;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Slow Wag 2");
        command->command = QLatin1String("TAILS2");
        command->category = QLatin1String("relaxed");
        command->duration = 7100;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Slow Wag 3");
        command->command = QLatin1String("TAILS3");
        command->category = QLatin1String("relaxed");
        command->duration = 8500;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Fast Wag");
        command->command = QLatin1String("TAILFA");
        command->category = QLatin1String("excited");
        command->duration = 9960;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Short Wag");
        command->command = QLatin1String("TAILSH");
        command->category = QLatin1String("excited");
        command->duration = 7460;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Happy Wag");
        command->command = QLatin1String("TAILHA");
        command->category = QLatin1String("excited");
        command->duration = 8860;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Stand up!");
        command->command = QLatin1String("TAILER");
        command->category = QLatin1String("excited");
        command->duration = 5800;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Tremble 1");
        command->command = QLatin1String("TAILT1");
        command->category = QLatin1String("tense");
        command->duration = 4060;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Tremble 2");
        command->command = QLatin1String("TAILT2");
        command->category = QLatin1String("tense");
        command->duration = 5550;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Tremble Erect");
        command->command = QLatin1String("TAILET");
        command->category = QLatin1String("tense");
        command->duration = 4730;
        command->minimumCooldown = 1000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("High Wag");
        command->command = QLatin1String("TAILEP");
        command->category = QLatin1String("tense");
        command->duration = 9760;
        command->minimumCooldown = 1000;
        d->commands << command;

        command = new CommandInfo();
        command->name = QLatin1String("BLINK");
        command->command = QLatin1String("LEDREC");
        command->category = QLatin1String("lights");
        command->duration = 10000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("TRIANGLE");
        command->command = QLatin1String("LEDTRI");
        command->category = QLatin1String("lights");
        command->duration = 10000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("SAWTOOTH");
        command->command = QLatin1String("LEDSAW");
        command->category = QLatin1String("lights");
        command->duration = 10000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("MORSE");
        command->command = QLatin1String("LEDSOS");
        command->category = QLatin1String("lights");
        command->duration = 10000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("BEACON");
        command->command = QLatin1String("LEDBEA");
        command->category = QLatin1String("lights");
        command->duration = 10000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("FLAME");
        command->command = QLatin1String("LEDFLA");
        command->category = QLatin1String("lights");
        command->duration = 10000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("STROBE");
        command->command = QLatin1String("LEDSTR");
        command->category = QLatin1String("lights");
        command->duration = 10000;
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Off");
        command->command = QLatin1String("LEDOFF");
        command->category = QLatin1String("");
        command->duration = 300;
        d->commands << command;
    }
    endResetModel();
    emit tailVersionChanged();
}

QString TailCommandModel::tailVersion() const
{
    return d->tailVersion;
}

TailCommandModel::CommandInfo * TailCommandModel::getCommand(int index) const
{
    if(index >= 0 && index < d->commands.count()) {
        return d->commands[index];
    }
    return nullptr;
}

TailCommandModel::CommandInfo * TailCommandModel::getCommand(QString command) const
{
    TailCommandModel::CommandInfo* cmd(nullptr);
    for(TailCommandModel::CommandInfo* current : d->commands) {
        if(current->command == command) {
            cmd = current;
            break;
        }
    }
    return cmd;
}

TailCommandModel::CommandInfo * TailCommandModel::getRandomCommand(QStringList includedCategories) const
{
    if(d->commands.count() > 0) {
        TailCommandModel::CommandInfoList pickFrom;
        if(includedCategories.isEmpty()) {
            pickFrom.append(d->commands);
        }
        else {
            for(TailCommandModel::CommandInfo* command : d->commands) {
                if(includedCategories.contains(command->category)) {
                    pickFrom << command;
                }
            }
        }
        return pickFrom.at(QRandomGenerator::global()->bounded(pickFrom.count()));
    }
    return nullptr;
}
