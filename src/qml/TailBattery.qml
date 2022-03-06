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

import QtQuick 2.7
import QtQuick.Controls 2.0

import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0

Column {
    id: batteryLayout
    enabled: visible;
    visible: height > 0;
    height: batteryRep.count * Kirigami.Units.iconSizes.small;
    Repeater {
        id: batteryRep;
        model: FilterProxyModel {
            id: deviceFilterProxy;
            sourceModel: DeviceModel;
            filterRole: 262; // the isConnected role
            filterBoolean: true;
        }
        Item {
            id: batteryDelegate
            height: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 3;
            width: batteryLayout.width
            property int batteryLevel: model.batteryLevel !== undefined ? model.batteryLevel : 0
            property int batteryLevelPercent: model.batteryLevelPercent !== undefined ? model.batteryLevelPercent : 0
            property int chargingState: model.chargingState !== undefined ? model.chargingState : 0
            Label {
                id: batteryLabel;
                anchors {
                    top: parent.top;
                    bottom: parent.bottom;
                }
                width: paintedWidth
                verticalAlignment: Text.AlignVCenter
                text: typeof model.name !== "undefined" ? model.name : ""
            }
            Label {
                id: commandLabel;
                anchors {
                    left: batteryLabel.right;
                    leftMargin: Kirigami.Units.smallSpacing;
                    top: parent.top;
                    bottom: parent.bottom;
                }
                width: paintedWidth
                verticalAlignment: Text.AlignVCenter
                text: typeof model.activeCommandTitles !== "undefined" ? model.activeCommandTitles : ""
                opacity: text === "" ? 0 : 0.5
                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; } }
            }

            Image {
                id: batteryIcon
                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                    margins: 1
                }
                width: height;
                property string chargingString: batteryDelegate.chargingState > 0 ? "-charging.svg" : ".svg";
                source: {
                    switch(batteryDelegate.batteryLevel) {
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
            }
            Label {
                anchors {
                    right: batteryIcon.left;
                    rightMargin: Kirigami.Units.smallSpacing;
                    top: batteryIcon.top;
                    bottom: batteryIcon.bottom;
                }
                verticalAlignment: Text.AlignVCenter
                text: batteryDelegate.batteryLevelPercent + "%";
            }
        }
    }
}
