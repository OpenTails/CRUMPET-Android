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
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    id: root;
    objectName: "moveListEditor";
    title: moveListName;
    property string moveListName;
    onMoveListNameChanged: {
        AppSettings.setActiveMoveList(moveListName);
    }

    actions {
        main: Kirigami.Action {
            text: qsTr("Add Move To List");
            icon.name: ":/org/kde/kirigami/icons/list-add.svg";
            onTriggered: {
                pickACommand.insertAt = moveListView.count;
                pickACommand.pickCommand();
            }
        }
        right: Kirigami.Action {
            text: qsTr("Add Pause To List");
            icon.name: ":/org/kde/kirigami/icons/accept_time_event.svg";
            onTriggered: {
                pickAPause.insertAt = moveListView.count;
                pickAPause.pickDuration();
            }
        }
    }
    Component {
        id: moveListDelegate;
        Kirigami.SwipeListItem {
            id: listItem;
            property variant command: {
                "name": "",
                "command": "",
                "minimumCooldown": 0,
                "duration": 0
            }
            Component.onCompleted: {
                Digitail.Utilities.getCommand(modelData);
            }
            property string title: command["name"];
            QQC2.Label {
                // Silly, yes, but we can't put it at the proper root of SwipeListItem, as it only wants QQuickItems there
                Connections {
                    target: Digitail.Utilities;
                    onCommandGotten: {
                        if(command.command === modelData) {
                            listItem.command = command;
                        }
                    }
                }
                text: command["minimumCooldown"] > 0
                        ? qsTr("%1<br/><small>%2 seconds</small><br/><small>Additional cooldown: %3 seconds</small>").arg(title).arg(command["duration"] / 1000).arg(command["minimumCooldown"] / 1000)
                        : qsTr("%1<br/><small>%2 seconds</small>").arg(title).arg(command["duration"] / 1000)
            }
            onClicked: { }
            actions: [
                Kirigami.Action {
                    text: qsTr("Add after");
                    icon.name: ":/org/kde/kirigami/icons/list-add.svg";
                    onTriggered: {
                        pickACommand.insertAt = index + 1;
                        pickACommand.pickCommand();
                    }
                },
                Kirigami.Action {
                    text: qsTr("Add pause after");
                    icon.name: ":/org/kde/kirigami/icons/accept_time_event.svg";
                    onTriggered: {
                        pickAPause.insertAt = index + 1;
                        pickAPause.pickDuration();
                    }
                },
                Kirigami.Action { },
                Kirigami.Action {
                    text: qsTr("Remove from list");
                    icon.name: ":/org/kde/kirigami/icons/list-remove.svg";
                    onTriggered: { }
                }
            ]
        }
    }
    ListView {
        id: moveListView;
        delegate: moveListDelegate;
        model: AppSettings.moveList
    }

    Kirigami.OverlaySheet {
        id: pickAPause;
        property int insertAt;
        onDurationPicked: {
            AppSettings.addMoveListEntry(insertAt, "pause:" + duration);
            pickACommand.close();
        }
        signal durationPicked(int duration);
        function pickDuration() {
            durationSlider.value = 10;
            open();
        }
        header: Kirigami.Heading {
            text: qsTr("Pick the duration of your pause");
        }
        Column {
            QQC2.Slider {
                id: durationSlider;
                width: root.width - Kirigami.Units.largeSpacing * 4;
                from: 1;
                to: 300;
                stepSize: 1;
                snapMode: QQC2.Slider.SnapAlways;
            }
            QQC2.Label {
                width: root.width - Kirigami.Units.largeSpacing * 4;
                text: qsTr("%1 seconds").arg(durationSlider.value);
            }
            QQC2.Button {
                text: qsTr("Add this pause");
                onClicked: {
                    pickAPause.durationPicked(durationSlider.value);
                }
            }
        }
    }

    PickACommandSheet {
        id: pickACommand;
        property int insertAt;
        onCommandPicked: {
            AppSettings.addMoveListEntry(insertAt, command);
            pickACommand.close();
        }
    }
}
