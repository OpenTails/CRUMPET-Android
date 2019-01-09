/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.4 as Kirigami

Kirigami.Card {
    property QtObject connectionManager: null;
    opacity: connectionManager.isConnected ? 0 : 1;
    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
    width: parent.width;
    header: Kirigami.Heading {
        text: qsTr("Not Connected");
        level: 2
        BusyIndicator {
            anchors {
                top: parent.top;
                right: parent.right;
                margins: Kirigami.Units.smallSpacing;
            }
            opacity: running;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            running: connectionManager.discoveryRunning;
        }
    }
    contentItem: Label {
        wrapMode: Text.Wrap;
        text: qsTr("You are not currently connected to your tail...");
    }
    actions: [
        Kirigami.Action {
            text: connectionManager.isConnected ? "Disconnect" : "Connect";
            icon.name: connectionManager.isConnected ? ":/org/kde/kirigami/icons/network-disconnect.svg" : ":/org/kde/kirigami/icons/network-connect.svg";
            onTriggered: {
                if(connectionManager.isConnected) {
                    connectionManager.disconnectDevice();
                }
                else {
                    connectToTail.open();
                }
            }
        }
    ]
}
