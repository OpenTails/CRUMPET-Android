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

#include "tailcommandmodel.h"

#include <QDebug>

class TailCommandModel::Private
{
public:
    Private() {}
    ~Private() {}

    QList<TailCommandModel::CommandInfo*> commands;
};

TailCommandModel::TailCommandModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
}

TailCommandModel::~TailCommandModel()
{
    delete d;
}

QHash< int, QByteArray > TailCommandModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[Command] = "command";
    roles[IsRunning] = "isRunning";
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
// TAIL MOVES----------
// TAILS1 - SLOW_WAG1
// TAILS2 - SLOW_WAG2
// TAILS3 - SLOW_WAG3
// TAILFA - FAST_WAG
// TAILSH - SHORT_WAG
// TAILHA - HAPPY_WAG
// TAILER - ERECT
// TAILEP - ERECT_PULSE
// TAILT1 - TREMBLE1
// TAILT2 - TREMBLE2
// TAILET - ERECT_TREM
//
// LEDS PATTERNS ----------
// LEDOFF - Leds off
// LEDREC - intermittent (on/off)
// LEDTRI - triangular (fade in/out)
// LEDSAW - sawtooth (fade in/off)
// LEDSOS - morse SOS
// LEDFLA - flame simulation
// LEDSTR - strobo
//
// OTHER COMMANDS-------
// VER - returns the firmware version number
// PING - returns OK
// SHUTDOWN - powers off the unit (will lose the connection!)
// BATT - returns Bn (n= 0,1,2,3,4 number of 'bars')

    beginResetModel();
    qDeleteAll(d->commands);
    d->commands.clear();
    if (version == QLatin1String()) {
        // Herpetyderp, no version, what's this?
    }
    else /* if (version == QLatin1String("the version for this tail version...")) */ {
        CommandInfo* command = new CommandInfo();
        command->name = QLatin1String("Tail Home Position");
        command->command = QLatin1String("TAILHM");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Slow Wag 1");
        command->command = QLatin1String("TAILS1");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Slow Wag 2");
        command->command = QLatin1String("TAILS2");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Slow Wag 3");
        command->command = QLatin1String("TAILS3");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Fast Wag");
        command->command = QLatin1String("TAILFA");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Short Wag");
        command->command = QLatin1String("TAILSH");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Happy Wag");
        command->command = QLatin1String("TAILHA");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Erect Tail");
        command->command = QLatin1String("ERECT");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Pulsing Erect Tail");
        command->command = QLatin1String("ERECT_PULSE");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Trembling 1");
        command->command = QLatin1String("TAILT1");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Trembling 2");
        command->command = QLatin1String("TAILT2");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("Trembling Erect Tail");
        command->command = QLatin1String("TAILET");
        d->commands << command;

        command = new CommandInfo();
        command->name = QLatin1String("LED Off");
        command->command = QLatin1String("LEDOFF");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("LED 1 Second Interval");
        command->command = QLatin1String("LEDREC");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("LED Triangular Fade");
        command->command = QLatin1String("LEDTRI");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("LED Sawtooth Fade");
        command->command = QLatin1String("LEDSAW");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("LED Beacon");
        command->command = QLatin1String("LEDBEA");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("LED Morse SOS");
        command->command = QLatin1String("LEDSOS");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("LED Flame Simulation");
        command->command = QLatin1String("LEDFLA");
        d->commands << command;
        command = new CommandInfo();
        command->name = QLatin1String("LED Strobe Effect");
        command->command = QLatin1String("LEDSTR");
        d->commands << command;
    }
    endResetModel();
}
