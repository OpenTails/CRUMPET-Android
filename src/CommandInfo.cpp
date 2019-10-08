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

#include "CommandInfo.h"

CommandInfo::CommandInfo()
{ }

CommandInfo::CommandInfo(const CommandInfo& other)
    : name(other.name)
    , command(other.command)
    , category(other.category)
    , duration(other.duration)
    , minimumCooldown(other.minimumCooldown)
    , isRunning(other.isRunning)
{ }

CommandInfo::CommandInfo(CommandInfo && other)
    : name(other.name)
    , command(other.command)
    , category(other.category)
    , duration(other.duration)
    , minimumCooldown(other.minimumCooldown)
    , isRunning(other.isRunning)
{
}

CommandInfo& CommandInfo::operator=(CommandInfo && other)
{
    name = other.name;
    command = other.command;
    category = other.category;
    duration = other.duration;
    minimumCooldown = other.minimumCooldown;
    isRunning = other.isRunning;
    return *this;
}

CommandInfo& CommandInfo::operator=(const CommandInfo& other)
{
    name = other.name;
    command = other.command;
    category = other.category;
    duration = other.duration;
    minimumCooldown = other.minimumCooldown;
    isRunning = other.isRunning;
    return *this;
}

CommandInfo::~CommandInfo()
{ }

void CommandInfo::clear()
{
    name.clear();
    command.clear();
    category.clear();
    duration = 0;
    minimumCooldown = 0;
    isRunning = false;
}

bool CommandInfo::compare(const CommandInfo& other) const
{
    // Not comparing isRunning, as that isn't necessarily quite as true...
    return (
        name == other.name &&
        command == other.command &&
        category == other.category &&
        duration == other.duration &&
        minimumCooldown == other.minimumCooldown
    );
}

bool CommandInfo::isValid() const
{
    if (name.isEmpty() || command.isEmpty() || category.isEmpty() || duration < 1) {
        return false;
    }
    return true;
}
