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
    id: root;
    property QtObject connectionManager: null;
    opacity: connectionManager.isConnected ? 0 : 1;
    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
    width: parent.width;
    header: Kirigami.Heading {
        text: connectionManager.discoveryRunning ? qsTr("Not Connected (searching...)") : qsTr("Not Connected");
        level: 2
        padding: Kirigami.Units.smallSpacing;
        BusyIndicator {
            anchors {
                top: parent.top;
                right: parent.right;
            }
            height: parent.height - 2 * Kirigami.Units.smallSpacing;
            width: height;
            opacity: running;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            running: connectionManager.discoveryRunning;
        }
    }
    contentItem: Label {
        wrapMode: Text.Wrap;
        text: connectionManager.discoveryRunning
            ? qsTr("You are not currently connected to your tail, and we are looking for it right now. Please ensure your tail is nearby and turned on.") + " " +
                (connectionManager.deviceModel.count === 0
                ? qsTr("We have not found any tails yet.")
                : qsTr("We have found %1 tails so far.").arg(connectionManager.deviceModel.count))
            : (connectionManager.deviceModel.count > 1
                ? qsTr("You are not currently connected to your tail, and we have found %1 tails. Please push Show available tails below to see the available tails.").arg(connectionManager.deviceModel.count)
                : qsTr("You are not currently connected to your tail, but we have found one tail. Please push Connect to connect to that tail now."))
            ;
    }
    actions: [
        Kirigami.Action {
            text: connectionManager.deviceModel.count > 1 ? qsTr("Show available tails...") : (connectionManager.deviceModel.count === 0 ? qsTr("Searching...") : qsTr("Connect now"));
            enabled: connectionManager.deviceModel.count > 0;
            icon.name: connectionManager.isConnected ? ":/org/kde/kirigami/icons/network-disconnect.svg" : ":/org/kde/kirigami/icons/network-connect.svg";
            onTriggered: {
                if(connectionManager.deviceModel.count === 1) {
                    // Calling this will stop the discovery immediately and connect to the one tail that we've found
                    connectionManager.stopDiscovery();
                }
                else {
                    connectToTail.open();
                }
            }
        }
    ]
}
