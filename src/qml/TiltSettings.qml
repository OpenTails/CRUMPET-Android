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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.ScrollablePage {
    id: component;
    objectName: "tiltSettings";
    title: i18nc("Title for the page for selecting what should happen when a piece of gear detects a tilting motion", "Tilt Settings");
    actions: [
        Kirigami.Action {
            text: i18nc("Button for returning Gear to the home position, on the page for selecting what should happen when a piece of gear detects a tilting motion", "Home Position");
            icon.name: "go-home";
            displayHint: Kirigami.DisplayHint.KeepVisible;
            onTriggered: {
                Digitail.BTConnectionManager.sendMessage("TAILHM", []);
            }
        }
    ]

    ColumnLayout {
        width: component.width - Kirigami.Units.largeSpacing * 4
        InfoCard {
            text: i18nc("Info card for the page for selecting what should happen when a piece of gear detects a tilting motion", "Turn on tilting detection by ticking the box beside the name of the gear you want to detect them on. To decide what to do when a particular kind of tilting is detected, pick the moves and what bits of gear you want to send that move to if you have more than one (otherwise we'll just send it to itself).");
        }
        Repeater {
            model: Digitail.FilterProxyModel {
                sourceModel: connectedDevicesModel;
                filterRole: Digitail.DeviceModelTypes.HasTilt;
                filterBoolean: true;
            }
            delegate: ColumnLayout {
                id: deviceCard
                property string deviceID: model.deviceID
                property var gestureEventValues: model.gestureEventValues
                property var gestureEventTitles: model.gestureEventTitles
                property var gestureEventCommands: model.gestureEventCommands
                property var gestureEventDevices: model.gestureEventDevices
                property var supportedTiltEvents: model.supportedTiltEvents
                Layout.fillWidth: true
                Rectangle {
                    Layout.fillWidth: true
                    Layout.minimumHeight: 1
                    Layout.maximumHeight: 1
                    color: Kirigami.Theme.textColor
                    visible: index > 0
                }
                BasicListItem {
                    Layout.fillWidth: true
                    text: model.name
                    icon.source: model.tiltEnabled > 0 ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
                    bold: true
                    onClicked: {
                        Digitail.BTConnectionManager.setDeviceTiltState(model.deviceID, !model.tiltEnabled);
                    }
                    PickACommandSheet {
                        id: pickACommand;

                        property string deviceID;
                        property int gestureId;

                        onCommandPicked: function(command, destinations) {
                            Digitail.BTConnectionManager.setDeviceGestureEventCommand(pickACommand.deviceID, pickACommand.gestureId, destinations, command);
                            pickACommand.close();
                        }
                    }
                }
                Repeater {
                    model: deviceCard.gestureEventValues
                    delegate: RowLayout {
                        id: gestureDelegate;
                        Layout.fillWidth: true;
                        property int gestureId: modelData
                        property string command: deviceCard.gestureEventCommands != undefined ? deviceCard.gestureEventCommands[model.index] : ""
                        visible: deviceCard.supportedTiltEvents != undefined ? deviceCard.supportedTiltEvents.includes(gestureId) : false
                        Text {
                            Layout.fillWidth: true;
                            text: deviceCard.gestureEventTitles != undefined ? deviceCard.gestureEventTitles[model.index] : ""
                        }
                        Button {
                            text: gestureDelegate.command === "" ? i18nc("Default text for the button for picking a command, for when no command has been selected, on the page for selecting what should happen when a piece of gear detects a tilting motion", "(no command)"): gestureDelegate.command;
                            onClicked: {
                                pickACommand.deviceID = deviceCard.deviceID
                                pickACommand.gestureId = gestureDelegate.gestureId;
                                pickACommand.pickCommand();
                            }
                        }
                        ToolButton {
                            Layout.alignment: Qt.AlignVCenter
                            height: parent.height - Kirigami.Units.smallSpacing * 2;
                            width: height;
                            icon.name: "edit-clear"
                            visible: gestureDelegate.command !== "";
                            onClicked: {
                                Digitail.BTConnectionManager.setDeviceGestureEventCommand(deviceCard.deviceID, gestureDelegate.gestureId, "", "");
                            }
                        }
                    }
                }
            }
        }
    }
}
