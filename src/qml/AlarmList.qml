/*
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import QtQuick.Layouts
import org.thetailcompany.digitail as Digitail

Kirigami.ScrollablePage {
    id: root;

    objectName: "alarmList";
    title: i18nc("Title of the page which lists the alarms the user has set", "Alarms");

    actions: [
        Kirigami.Action {
            text: i18nc("Text for a button which will adds a new alarm to the list", "Add New Alarm");
            icon.name: "list-add";
            displayHint: Kirigami.DisplayHint.KeepVisible;
            onTriggered: {
                namePicker.pickName();
            }
        }
    ]

    Connections {
        target: Digitail.AppSettings

        function onAlarmExisted() {
            showMessageBox(i18nc("Title for a message box warning the user an alarm with the chosen name already exists", "Select another name"),
                           i18nc("Main text for a message box warning the user an alarm with the chosen name already exists", "Unable to add the alarm because we already have an alarm with the same name.\n\nPlease select another name for the alarm."));
        }

        function onAlarmNotExisted(name) {
            showMessageBox(i18nc("Title for a message box informing the user the alarm they attempted to remove does not exist", "Alarm was removed"),
                           i18nc("Main text for a message box informing the user the alarm they attempted to remove does not exist", "Unable to find an alarm with the name '%1'. Maybe another application instance has removed it.", name));
        }
    }

    Component {
        id: alarmListDelegate;

        Kirigami.SwipeListItem {
            id: listItem;

            property var dateTime: modelData["time"]

            ColumnLayout {
                QQC2.Label {
                    text: modelData["name"]
                }

                QQC2.Label {
                    text: Qt.formatDate(dateTime, Qt.DefaultLocaleLongDate)
                          + ", "
                          + (locale.amText ? Qt.formatTime(dateTime, "hh:mm AP") : Qt.formatTime(dateTime, "hh:mm"))
                }
            }

            onClicked: {
                console.debug("Append this list, yo! ...maybe confirm, because it's a bit lots");
            }

            actions: [
                Kirigami.Action {
                    text: i18nc("Text for an action which allows the user to pick what command the alarm will cause to be sent", "Edit Alarm Commands");
                    icon.name: "document-edit";
                    displayHint: Kirigami.DisplayHint.KeepVisible;
                    onTriggered: {
                        pageStack.push(editorPage, { alarm: modelData });
                    }
                },

                Kirigami.Action {
                    text: i18nc("Text for an action which allows the user to pick what time the alarm should fire", "Set Time To Alarm");
                    icon.name: "accept_time_event";
                    displayHint: Kirigami.DisplayHint.KeepVisible;
                    onTriggered: {
                        Digitail.AppSettings.setActiveAlarmName(modelData["name"]);

                        datePicker.showDatePicker(dateTime, function(date) {
                            var originDate = dateTime;
                            date.setHours(originDate.getHours());
                            date.setMinutes(originDate.getMinutes());

                            timePicker.showTimePicker(originDate.getHours(), originDate.getMinutes(), function(hours, minutes) {
                                date.setHours(hours);
                                date.setMinutes(minutes);
                                Digitail.AppSettings.setAlarmTime(date);
                                Digitail.AppSettings.setActiveAlarmName("");
                            });
                        })
                    }
                },

                Kirigami.Action {
                    text: i18nc("Text for an action which allows the user to delete an alarm", "Delete this Alarm");
                    icon.name: "list-remove";
                    displayHint: Kirigami.DisplayHint.KeepVisible;
                    onTriggered: {
                        showMessageBox(i18nc("Title for a message box which allows the user to delete a specific alarm", "Remove the Alarm"),
                                       i18nc("Main text for a message box which allows the user to delete a specific alarm", "Are you sure that you want to remove the alarm '%1'?", modelData["name"]),
                                       function () {
                                           Digitail.AppSettings.removeAlarm(modelData["name"]);
                                       });
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
        model: Digitail.AppSettings.alarmList;
        delegate: alarmListDelegate;
        header: InfoCard {
            text: i18nc("Descriptive text which explains what the functionality is of the alarm page", "Set an alarm here. Pick a date and time for your alarm, and then add one or more moves you want to perform when you hit that time.");
        }
    }

    NamePicker {
        id: namePicker;

        description: i18nc("Descriptive text for a popup which allows the user to create a new, named alarm", "Enter a name to use for your new alarm and click Create");
        placeholderText: i18nc("Placeholder text for the text box in which the name of a new alarm must be written", "Enter your alarm name here");
        buttonOkText: i18nc("Text on a button which causes an alarm to be created", "Create");

        onNamePicked: {
            Digitail.AppSettings.addAlarm(name);
            namePicker.close();
        }
    }

    DatePicker {
        id: datePicker
    }

    TimePicker {
        id: timePicker
    }
}
