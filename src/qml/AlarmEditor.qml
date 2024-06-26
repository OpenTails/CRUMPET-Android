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

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

BaseCommandListEditor {
    property var alarm: null;
    readonly property string alarmName: alarm ? alarm["name"] : "";

    objectName: "alarmEditor";
    title: alarmName;
    model: Digitail.AppSettings.activeAlarm["commands"];

    onAlarmNameChanged: {
        Digitail.AppSettings.setActiveAlarmName(alarmName);
    }

    onInsertCommand: function(insertAt, command, destinations) {
        if (!alarm) {
            return;
        }

        Digitail.AppSettings.addAlarmCommand(insertAt, command, destinations);
    }

    onRemoveCommand: function(index) {
        if (!alarm) {
            return;
        }

        Digitail.AppSettings.removeAlarmCommand(index);
    }

    Component.onCompleted: {
        Digitail.AppSettings.setActiveAlarmName(alarmName);
    }
}
