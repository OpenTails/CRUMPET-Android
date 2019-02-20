/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   This file based on sample code from Kirigami
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

import QtQuick 2.7
import QtQuick.Controls 2.4 as QQC2
import org.kde.kirigami 2.6 as Kirigami

Kirigami.ScrollablePage {
    id: root;

    objectName: "alarmList";
    title: qsTr("Alarms");

    actions {
        main: Kirigami.Action {
            text: qsTr("Add New Alarm");
            icon.name: ":/org/kde/kirigami/icons/list-add.svg";
            onTriggered: {
                namePicker.pickName();
            }
        }
    }

    Connections {
        target: AppSettings

        onAlarmExisted: {
            showMessageBox(qsTr("Unable to add the alarm because we already have an alarm with the same name.\n\nPlease select another name for the alarm."));
        }

        onAlarmNotExisted: {
            showMessageBox(qsTr("Unable to find an alarm with the name '%1'. Maybe another application instance has removed it.").arg(name));
        }
    }

    Component {
        id: alarmListDelegate;

        Kirigami.SwipeListItem {
            id: listItem;

            QQC2.Label {
                text: modelData["name"]
            }

            onClicked: {
                console.debug("Append this list, yo! ...maybe confirm, because it's a bit lots");
            }

            actions: [
                Kirigami.Action {
                    text: qsTr("Edit Alarm");
                    icon.name: ":/org/kde/kirigami/icons/document-edit.svg";
                    onTriggered: {
                        pageStack.push(editorPage, { alarm: modelData });
                    }
                },

                Kirigami.Action { },

                Kirigami.Action {
                    text: qsTr("Delete this Alarm");
                    icon.name: ":/org/kde/kirigami/icons/list-remove.svg";
                    onTriggered: {
                        console.debug("Delete this alarm, but ask first...");
                    }
                }
            ]
        }
    }

    Component {
        id: editorPage;
        AlarmEditor { }
    }

    ListView {
        model: AppSettings.alarmList;
        delegate: alarmListDelegate;
    }

    NamePicker {
        id: namePicker;

        description: qsTr("Enter a name to use for your new alarm and click Create");
        placeholderText: qsTr("Enter your alarm name here");
        buttonOkText: qsTr("Create");

        onNamePicked: {
            AppSettings.addAlarm(name);
            namePicker.close();
        }
    }
}
