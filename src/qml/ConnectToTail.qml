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
import org.kde.kirigami 2.4 as Kirigami

Kirigami.OverlaySheet {
    id: sheet;
    property QtObject pageToPush: null;
    showCloseButton: true;
    signal attemptToConnect(string deviceID, QtObject pageToPush);
    header: RowLayout {
        implicitWidth: Kirigami.Units.gridUnit * 30
        Kirigami.Icon {
            source: ":/org/kde/kirigami/icons/network-connect.svg"
            width: Kirigami.Units.iconSizes.large
            height: width
        }
        Kirigami.Heading {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            wrapMode: Text.WordWrap
            text: "Connect To Tail"
        }
    }
    ListView {
        implicitWidth: Kirigami.Units.gridUnit * 30
        model: DeviceModel;
        delegate: Kirigami.AbstractListItem {
            height: Kirigami.Units.gridUnit * 6
            hoverEnabled: false
            //TODO: bug in overlaysheet
            rightPadding: Kirigami.Units.gridUnit * 1.5
            RowLayout {
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    QQC2.Label {
                        wrapMode: Text.WordWrap
                        text: model.name
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        wrapMode: Text.WordWrap
                        text: model.deviceID
                    }
                }
                QQC2.Button {
                    text: "Connect"
                    onClicked: {
                        sheet.close();
                        sheet.attemptToConnect(model.deviceID, pageToPush);
                        pageToPush = null;
                    }
                }
            }
        }
    }
}
