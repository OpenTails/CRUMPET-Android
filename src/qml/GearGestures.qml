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
            text: qsTr("Turn on one of the sensors below to make your gear react to gestures performed on this device, if there is nothing else going on (that is, no current commands, and an empty command queue). For example, make your ears perk up when the device recognises that is has been picked up, or start wagging when it detects that you have taken a step.");
        }
        Repeater {
            model: GestureDetectorModel;
            ColumnLayout {
                ColumnLayout {
                    visible: model.firstInSensor === undefined ? false : model.firstInSensor;
                    Layout.fillWidth: true;
                    Rectangle {
                        Layout.fillWidth: true;
                        height: 1;
                        color: Kirigami.Theme.textColor;
                        visible: model.index > 0
                    }
                    Kirigami.BasicListItem {
                        visible: model.firstInSensor === undefined ? false : model.firstInSensor;
                        Layout.fillWidth: true;
                        separatorVisible: false;
                        bold: true;
                        icon: model.sensorEnabled > 0 ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        label: model.sensorName === undefined ? "" : model.sensorName;
                        onClicked: { GestureController.setGestureSensorEnabled(model.index, !model.sensorEnabled); }
                    }
                    CheckBox {
                        Layout.fillWidth: true;
                        text: qsTr("Show On Welcome Page")
                        checked: model.sensorPinned === undefined ? false : model.sensorPinned
                        onClicked: GestureController.setGestureSensorPinned(model.index, !model.sensorPinned)
                    }
                }
                RowLayout {
                    id: gestureDelegate;
                    Layout.fillWidth: true;
                    Text {
                        Layout.fillWidth: true;
                        text: model.name === undefined ? "" : model.name;
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
                        icon.name: "edit-clear"
                        visible: model.command !== "";
                        onClicked: {
                            GestureController.setGestureDetails(model.index, "", "");
                        }
                    }
                    ToolButton {
                        Layout.alignment: Qt.AlignVCenter
                        height: parent.height - Kirigami.Units.smallSpacing * 2;
                        width: height;
                        icon.name: "document-revert"
                        visible: model.command === "" && model.defaultCommand !== "";
                        onClicked: {
                            GestureController.setGestureDetails(model.index, model.defaultCommand, "");
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
