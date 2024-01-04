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

import QtQuick 2.7
import QtQuick.Controls 2.0 as QQC2
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.OverlaySheet {
    id: sheet;
    property QtObject pageToPush: null;
    showCloseButton: true;
    signal attemptToConnect(string deviceID, QtObject pageToPush);
    header: RowLayout {
        implicitWidth: Kirigami.Units.gridUnit * 30
        Kirigami.Icon {
            source: "network-connect"
            width: Kirigami.Units.iconSizes.large
            height: width
        }
        Kirigami.Heading {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            wrapMode: Text.WordWrap
            text: "Connect To Gear"
        }
    }
    footer: RowLayout {
        implicitWidth: Kirigami.Units.gridUnit * 30
        QQC2.Button {
            Layout.fillWidth: true
            text: i18nc("Button for the action of connecting to all found devices, in the popup for connecting to devices", "Connect All")
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
        delegate: Kirigami.AbstractListItem {
            height: Kirigami.Units.gridUnit * 6
            hoverEnabled: false
            RowLayout {
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
                    text: model.isConnected ? 
                        i18nc("Button for the action of disconnecting a device, in the prompt for connecting a device", "Disconnect") : 
                        i18nc("Button for the action of connecting a device, in the prompt for connecting a device", "Connect")
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
