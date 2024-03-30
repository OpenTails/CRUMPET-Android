/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.OverlaySheet {
    id: sheet;
    property QtObject pageToPush: null;
    showCloseButton: true;
    parent: QQC2.Overlay.overlay
    title: i18nc("Title on the popup for connecting to devices", "Connect To Gear")
    signal attemptToConnect(string deviceID, QtObject pageToPush);
    footer: RowLayout {
        implicitWidth: Kirigami.Units.gridUnit * 30
        QQC2.Button {
            Layout.fillWidth: true
            text: i18nc("Button for the action of disconnecting from all currently connected devices, in the popup for connecting to devices", "Disconnect All")
            Digitail.FilterProxyModel {
                id: connectedDevices
                sourceModel: Digitail.DeviceModel;
                filterRole: Digitail.DeviceModelTypes.IsConnected;
                filterBoolean: true;
            }
            enabled: connectedDevices.count > 0
            onClicked: {
                disconnectionOptions.disconnectGear("");
                sheet.close();
            }
        }
        QQC2.Button {
            Layout.fillWidth: true
            text: i18nc("Button for the action of connecting to all found devices, in the popup for connecting to devices", "Connect All")
            enabled: connectedDevices.count < deviceList.count
            onClicked: {
                for(var i = 0; i < deviceList.count; ++i) {
                    var isConnected = Digitail.DeviceModel.data(Digitail.DeviceModel.index(i, 0), Digitail.DeviceModelTypes.IsConnected);
                    if (isConnected == false) {
                        var deviceID = Digitail.DeviceModel.data(Digitail.DeviceModel.index(i, 0), Digitail.DeviceModelTypes.DeviceID);
                        sheet.attemptToConnect(deviceID, sheet.pageToPush);
                        sheet.pageToPush = null;
                    }
                }
                sheet.close();
            }
        }
    }
    ListView {
        id: deviceList;
        implicitWidth: Kirigami.Units.gridUnit * 30
        model: Digitail.DeviceModel;
        delegate: QQC2.ItemDelegate {
            width: ListView.view.width
            height: Kirigami.Units.gridUnit * 6
            contentItem: RowLayout {
                Kirigami.Icon {
                    source: model.deviceIcon
                    Layout.fillHeight: true
                    Layout.maximumHeight: Kirigami.Units.iconSizes.medium
                    Layout.minimumWidth: height
                    Layout.maximumWidth: height
                    Layout.alignment: Qt.AlignVCenter
                    isMask: true
                    color: model.color !== undefined ? model.color : "transparent"
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    QQC2.Label {
                        wrapMode: Text.WordWrap
                        text: model.name ? model.name : ""
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        wrapMode: Text.WordWrap
                        text: model.deviceID ? model.deviceID : ""
                    }
                }
                QQC2.Button {
                    text: model.isConnecting
                        ? i18nc("A label in the prompt for connecting to a device showing that the device is currently connecting", "Connecting...")
                        : model.isConnected
                            ? i18nc("Button for the action of disconnecting a device, in the prompt for connecting a device", "Disconnect")
                            : i18nc("Button for the action of connecting a device, in the prompt for connecting a device", "Connect")
                    enabled: model.isConnecting == false
                    onClicked: {
                        sheet.close();
                        if (model.isConnected) {
                            disconnectionOptions.disconnectGear(model.deviceID);
                        } else {
                            sheet.attemptToConnect(model.deviceID, pageToPush);
                            pageToPush = null;
                        }
                    }
                }
            }
        }
    }
}
