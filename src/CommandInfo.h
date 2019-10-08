/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef COMMANDINFO_H
#define COMMANDINFO_H

#include <QList>

class CommandInfo {
public:
    CommandInfo();
    CommandInfo(const CommandInfo& other);
    CommandInfo(CommandInfo&& other);
    CommandInfo& operator=(const CommandInfo& other);
    CommandInfo& operator=(CommandInfo&& other);
    ~CommandInfo();

    QString name;
    QString command;
    QString category;
    int duration{0}; // milliseconds
    int minimumCooldown{0}; // milliseconds

    bool isRunning{false};

    void clear();
    bool compare(const CommandInfo& other) const;
    /**
     * Returns true if name, command, category, and duration are all set, otherwise false
     */
    bool isValid() const;
};
typedef QList<CommandInfo> CommandInfoList;

#endif//COMMANDINFO_H
