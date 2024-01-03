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
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    id: component;
    objectName: "tiltSettings";
    title: i18nc("Title for the page for selecting what should happen when a piece of gear detects a tilting motion", "Tilt Settings");
    actions {
        main: Kirigami.Action {
            text: i18nc("Button for returning Gear to the home position, on the page for selecting what should happen when a piece of gear detects a tilting motion", "Home Position");
            icon.name: "go-home";
            onTriggered: {
                Digitail.BTConnectionManager.sendMessage("TAILHM", []);
            }
        }
    }

    ColumnLayout {
        width: component.width - Kirigami.Units.largeSpacing * 4
        InfoCard {
            text: i18nc("Info card for the page for selecting what should happen when a piece of gear detects a tilting motion", "Turn on one of the sensors below to make your gear react to gestures performed on this device, if there is nothing else going on (that is, no current commands, and an empty command queue). For example, make your ears perk up when the device recognises that is has been picked up, or start wagging when it detects that you have taken a step.");
        }
        Repeater {
            model: Digitail.FilterProxyModel {
                sourceModel: connectedDevicesModel;
                filterRole: 282; // the hasTilt role
                filterBoolean: true;
            }
            delegate: SettingsCard {
                id: deviceCard
                headerText: model.name
                property string deviceID: model.deviceID
                property var gestureEventValues: model.gestureEventValues
                property var gestureEventTitles: model.gestureEventTitles
                property var gestureEventCommands: model.gestureEventCommands
                property var gestureEventDevices: model.gestureEventDevices
                contentItem: ColumnLayout {
                    Repeater {
                        model: deviceCard.gestureEventValues
                        delegate: RowLayout {
                            id: gestureDelegate;
                            Layout.fillWidth: true;
                            property int gestureId: modelData
                            property string command: deviceCard.gestureEventCommands != undefined ? deviceCard.gestureEventCommands[model.index] : ""
                            // Device gestures in the 10s are tilt ones, so show only those
                            visible: 9 < gestureId && gestureId < 20
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

    PickACommandSheet {
        id: pickACommand;

        property string deviceID;
        property int gestureId;

        onCommandPicked: {
            Digitail.BTConnectionManager.setDeviceGestureEventCommand(pickACommand.deviceID, pickACommand.gestureId, destinations, command);
            pickACommand.close();
        }
    }
}
