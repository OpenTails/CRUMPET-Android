/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

ColumnLayout {
    id: batteryLayout
    enabled: visible;
    visible: batteryRep.count > 0 || connectingRep.count > 0;
    Repeater {
        id: connectingRep;
        model: Digitail.FilterProxyModel {
            sourceModel: Digitail.DeviceModel;
            filterRole: Digitail.DeviceModelTypes.IsConnecting;
            filterBoolean: true;
        }
        ColumnLayout {
            RowLayout {
                Layout.fillWidth: true
                Kirigami.Icon {
                    source: model.deviceIcon
                    Layout.fillHeight: true
                    Layout.maximumHeight: Kirigami.Units.iconSizes.small
                    Layout.minimumWidth: height
                    Layout.maximumWidth: height
                    Layout.alignment: Qt.AlignVCenter
                    isMask: true
                    color: model.color !== undefined ? model.color : "transparent"
                }
                Label {
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    text: typeof model.name !== "undefined" ? model.name : ""
                }
                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignRight
                    text: i18nc("Text shown beside a spinner indicating that we are currently attempting to connect to a piece of gear", "Connecting")
                }
                BusyIndicator {
                    Layout.fillHeight: true
                    Layout.maximumHeight: Kirigami.Units.iconSizes.medium
                    Layout.minimumWidth: height
                    Layout.maximumWidth: height
                    running: true
                }
            }
        }
    }
    Repeater {
        id: batteryRep;
        model: Digitail.FilterProxyModel {
            sourceModel: Digitail.DeviceModel;
            filterRole: Digitail.DeviceModelTypes.IsConnected;
            filterBoolean: true;
        }
        ColumnLayout {
            RowLayout {
                id: batteryDelegate
                Layout.fillWidth: true
                property int batteryLevel: model.batteryLevel !== undefined ? model.batteryLevel : -1
                property int batteryLevelPercent: model.batteryLevelPercent !== undefined ? model.batteryLevelPercent : 0
                property int chargingState: model.chargingState !== undefined ? model.chargingState : 0
                Kirigami.Icon {
                    source: model.deviceIcon
                    Layout.fillHeight: true
                    Layout.maximumHeight: Kirigami.Units.iconSizes.small
                    Layout.minimumWidth: height
                    Layout.maximumWidth: height
                    Layout.alignment: Qt.AlignVCenter
                    isMask: true
                    color: model.color !== undefined ? model.color : "transparent"
                    TapHandler {
                        onTapped: {
                            namePicker.checkDeviceName(model.deviceID, true);
                        }
                    }
                }
                Label {
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    text: typeof model.name !== "undefined" ? model.name : ""
                    TapHandler {
                        onTapped: {
                            namePicker.checkDeviceName(model.deviceID, true);
                        }
                    }
                }
                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    text: typeof model.activeCommandTitles !== "undefined" ? model.activeCommandTitles : ""
                    opacity: text === "" ? 0 : 0.5
                    elide: Text.ElideRight;
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; } }
                }
                Image {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    Layout.minimumWidth: height;
                    Layout.maximumWidth: height;
                    property string chargingString: batteryDelegate.chargingState > 0 ? "-charging.svg" : ".svg";
                    source: {
                        switch(batteryDelegate.batteryLevel) {
                            case -1:
                            case 0:
                                return "qrc:/icons/breeze-internal/status/22/battery-000" + chargingString
                                break;
                            case 1:
                                return "qrc:/icons/breeze-internal/status/22/battery-020" + chargingString
                                break;
                            case 2:
                                return "qrc:/icons/breeze-internal/status/22/battery-050" + chargingString
                                break;
                            case 3:
                                return "qrc:/icons/breeze-internal/status/22/battery-070" + chargingString
                                break;
                            case 4:
                            case 5:
                            default:
                                return "qrc:/icons/breeze-internal/status/22/battery-100" + chargingString
                                break;
                        }
                    }
                    Image {
                        anchors {
                            fill: parent
                            margins: Kirigami.Units.largeSpacing
                        }
                        visible: batteryDelegate.batteryLevel == -1
                        source: "qrc:/icons/breeze-internal/actions/16/paint-unknown.svg"
                    }
                }
                Label {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    Layout.minimumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.medium;
                    verticalAlignment: Text.AlignVCenter
                    opacity: batteryDelegate.batteryLevel > -1
                    text: batteryDelegate.batteryLevelPercent + "%";
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: knownFirmwareMessageLayout.height + Kirigami.Units.smallSpacing * 2
                visible: knownFirmwareMessageLabel.text.length > 0
                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.Tooltip
                border {
                    width: 1
                    color: Kirigami.Theme.textColor
                }
                color: Kirigami.Theme.backgroundColor
                radius: Kirigami.Units.smallSpacing
                RowLayout {
                    id: knownFirmwareMessageLayout
                    anchors {
                        top: parent.top;
                        left: parent.left;
                        right: parent.right;
                        margins: Kirigami.Units.smallSpacing;
                    }
                    Kirigami.Icon {
                        Layout.fillHeight: true;
                        Layout.minimumWidth: Kirigami.Units.iconSizes.large;
                        Layout.maximumWidth: Kirigami.Units.iconSizes.large;
                        source: "qrc:/icons/breeze-internal/status/22/dialog-warning.svg"
                    }
                    Label {
                        id: knownFirmwareMessageLabel
                        Layout.fillHeight: true;
                        Layout.fillWidth: true;
                        wrapMode: Text.Wrap
                        color: Kirigami.Theme.textColor
                        text: typeof model.knownFirmwareMessage !== "undefined" ? knownFirmwareMessage : ""
                    }
                }
            }
        }
    }
}
