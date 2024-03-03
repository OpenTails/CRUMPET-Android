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
    objectName: "gearGestures";
    title: i18nc("Title for the page for selecting what should happen when the controlling device/phone detects a gesture", "Gear Gestures");
    actions: [
        Kirigami.Action {
            text: i18nc("Button for returning Gear to the home position, on the page for selecting what should happen when the controlling device/phone detects a gesture", "Home Position");
            icon.name: "go-home";
            displayHint: Kirigami.DisplayHint.KeepVisible;
            onTriggered: {
                Digitail.BTConnectionManager.sendMessage("TAILHM", []);
            }
        }
    ]
    property QtObject enabledGestures: Digitail.FilterProxyModel {
        sourceModel: Digitail.GestureDetectorModel;
        filterRole: Digitail.GestureDetectorModelTypes.SensorEnabledRole;
        filterBoolean: true;
    }

    ColumnLayout {
        width: component.width - Kirigami.Units.largeSpacing * 4
        InfoCard {
            text: i18nc("Info card for the page for selecting what should happen when the controlling device/phone detects a gesture", "Turn on one of the sensors below to make your gear react to gestures performed on this device, if there is nothing else going on (that is, no current commands, and an empty command queue). For example, make your ears perk up when the device recognises that is has been picked up, or start wagging when it detects that you have taken a step.");
        }
        Repeater {
            model: Digitail.GestureDetectorModel;
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
                    BasicListItem {
                        visible: model.firstInSensor === undefined ? false : model.firstInSensor;
                        Layout.fillWidth: true;
                        bold: true;
                        icon.source: model.sensorEnabled > 0 ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        text: model.sensorName === undefined ? "" : model.sensorName;
                        onClicked: {
                            if(!model.sensorEnabled && enabledGestures.count > 0) {
                                applicationWindow().showMessageBox(i18nc("Title for the warning for having enabled multiple gestures at the same time, on the page for selecting what should happen when the controlling device/phone detects a gesture","Multiple Gestures"),
                                    i18nc("Description for the warning for having enabled multiple gestures at the same time, on the page for selecting what should happen when the controlling device/phone detects a gesture", "You are attempting to turn on more than one gesture at the same time. This will occasionally cause problems, primarily by being confusing to manage (for example, turning on both Walking and Shake is likely to cause both to be detected). If you are sure you want to do this, tap OK, or tap Cancel to not enable this gesture."),
                                    function() {Digitail.GestureController.setGestureSensorEnabled(model.index, !model.sensorEnabled)});
                            } else {
                                Digitail.GestureController.setGestureSensorEnabled(model.index, !model.sensorEnabled);
                            }
                        }
                    }
                    CheckBox {
                        Layout.fillWidth: true;
                        text: i18nc("Description for the checkbox for showing a gesture on the Welcome Page, on the page for selecting what should happen when the controlling device/phone detects a gesture", "Show On Welcome Page")
                        checked: model.sensorPinned === undefined ? false : model.sensorPinned
                        onClicked: Digitail.GestureController.setGestureSensorPinned(model.index, !model.sensorPinned)
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
                        text: model.command === "" ? i18nc("Default text for the button for picking a command, for when no command has been selected, on the page for selecting what should happen when the controlling device/phone detects a gesture", "(no command)"): model.command;
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
                            Digitail.GestureController.setGestureDetails(model.index, "", "");
                        }
                    }
                    ToolButton {
                        Layout.alignment: Qt.AlignVCenter
                        height: parent.height - Kirigami.Units.smallSpacing * 2;
                        width: height;
                        icon.name: "document-revert"
                        visible: model.command === "" && model.defaultCommand !== "";
                        onClicked: {
                            Digitail.GestureController.setGestureDetails(model.index, model.defaultCommand, "");
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
            Digitail.GestureController.setGestureDetails(pickACommand.gestureIndex, command, destinations);
            pickACommand.close();
        }
    }
}
