/*
 *   Copyright 2020 Evgeni B<evgeni.biryuk.tail@gofree.club>
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

import QtQuick 2.11
import QtQuick.Controls 2.11
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami

Kirigami.ScrollablePage {
    id: component;
    objectName: "gearGestures";
    title: qsTr("Gear Gestures");
    actions {
        main: Kirigami.Action {
            text: qsTr("Home Position");
            icon.name: "go-home";
            onTriggered: {
                BTConnectionManager.sendMessage("TAILHM", []);
            }
        }
    }

    ColumnLayout {
        width: component.width - Kirigami.Units.largeSpacing * 4
        InfoCard {
            text: qsTr("Turn on Gesture Control to make your gear react to gestures performed on this device, if there is nothing else going on (that is, no current commands, and an empty command queue). For example, make your ears perk up when the device recognises that is has been picked up.");
            footer: CheckBox {
                text: qsTr("Enable Gesture Control")
                Layout.fillWidth: true
                checkable: false;
                checked: GestureController.enabled;

                onClicked: {
                    GestureController.enabled = !GestureController.enabled;
                }
            }
        }
        Repeater {
            model: GestureDetectorModel;
            ColumnLayout {
                Kirigami.Heading {
                    Layout.fillWidth: true;
                    visible: model.firstInSensor === undefined ? false : model.firstInSensor;
                    text: model.sensorName === undefined ? "" : model.sensorName;
                }
                RowLayout {
                    id: gestureDelegate;
                    Layout.fillWidth: true;
                    Text {
                        Layout.fillWidth: true;
                        text: qsTr("%1 gesture:").arg(model.name);
                    }
                    Button {
                        text: model.command === "" ? qsTr("(no command)"): model.command;
                        onClicked: {
                            pickACommand.gestureIndex = model.index;
                            pickACommand.pickCommand();
                        }
                    }
                    ToolButton {
                        Layout.alignment: Qt.AlignVCenter
                        height: parent.height - Kirigami.Units.smallSpacing * 2;
                        width: height;
                        contentItem: Kirigami.Icon {
                            source: "edit-clear"
                        }
                        visible: model.command !== "";
                        onClicked: {
                            GestureController.setGestureDetails(model.index, "", "");
                        }
                    }
                }
            }
        }
    }

    PickACommandSheet {
        id: pickACommand;

        property int gestureIndex;

        onCommandPicked: {
            GestureController.setGestureDetails(pickACommand.gestureIndex, command, destinations);
            pickACommand.close();
        }
    }
}
