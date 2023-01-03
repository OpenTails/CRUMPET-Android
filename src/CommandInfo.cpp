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
    , group(other.group)
    , isRunning(other.isRunning)
    , isAvailable(other.isAvailable)
{ }

CommandInfo::CommandInfo(CommandInfo && other) noexcept
    : name(std::move(other.name))
    , command(std::move(other.command))
    , category(std::move(other.category))
    , duration(std::move(other.duration))
    , minimumCooldown(std::move(other.minimumCooldown))
    , group(std::move(other.group))
    , isRunning(std::move(other.isRunning))
    , isAvailable(std::move(other.isAvailable))
{
}

CommandInfo& CommandInfo::operator=(CommandInfo && other) noexcept
{
    name = std::move(other.name);
    command = std::move(other.command);
    category = std::move(other.category);
    duration = std::move(other.duration);
    minimumCooldown = std::move(other.minimumCooldown);
    group = std::move(other.group);
    isRunning = std::move(other.isRunning);
    isAvailable = std::move(other.isAvailable);
    return *this;
}

CommandInfo& CommandInfo::operator=(const CommandInfo& other)
{
    name = other.name;
    command = other.command;
    category = other.category;
    duration = other.duration;
    minimumCooldown = other.minimumCooldown;
    group = other.group;
    isRunning = other.isRunning;
    isAvailable = other.isAvailable;
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
    group = 0;
    isRunning = false;
    isAvailable = true;
}

bool CommandInfo::compare(const CommandInfo& other) const
{
    // Not comparing isRunning and isAvailable, as that isn't necessarily quite as true...
    return (
        name == other.name &&
        command == other.command &&
        category == other.category &&
        duration == other.duration &&
        minimumCooldown == other.minimumCooldown &&
        group == other.group
    );
}

bool CommandInfo::equivalent(const CommandInfo &other) const
{
    // Only checking the name, command, category, and group, as duration and cooldown is different per-device. Used for grouping purposes
    return (
        name == other.name &&
        command == other.command &&
        category == other.category &&
        group == other.group
    );
}

bool CommandInfo::isValid() const
{
    if (name.isEmpty() || command.isEmpty() || category.isEmpty() || duration < 1) {
        return false;
    }
    return true;
}
