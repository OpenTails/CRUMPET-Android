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
import org.kde.kirigami 2.5 as Kirigami

Kirigami.Card {
    id: root;
    property QtObject connectionManager: null;
    opacity: connectionManager.isConnected ? 0 : 1;
    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
    width: parent.width;
    header: Kirigami.Heading {
        text: {
            if (connectionManager.discoveryRunning === true) {
                return qsTr("Searching for your tail...");
            }
            else {
                if (connectionManager.deviceModel.count === 0) {
                    return qsTr("No tails found");
                }
                else if (connectionManager.deviceModel.count === 1) {
                    return qsTr("One tail available");
                }
                else {
                    return qsTr("Multiple tails available");
                }
            }
        }
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
        text: {
            if (connectionManager.discoveryRunning === true) {
                if (connectionManager.deviceModel.count === 0) {
                    return qsTr("No tails found yet, still looking...");
                }
                else if (connectionManager.deviceModel.count > 1) {
                    return qsTr("Found %1 tails so far. To see them, push \"Show available tails...\" below.").arg(connectionManager.deviceModel.count);
                }
                else {
                    return qsTr("1 tail found. Simply wait, or push \"Connect\" below to control it.");
                }
            }
            else {
                if (connectionManager.deviceModel.count === 0) {
                    return qsTr("We were unable to find any tails. Please ensure that it is nearby and switched on.");
                }
                else if (connectionManager.deviceModel.count > 1) {
                    return qsTr("You are not currently connected to your tail, and we have found %1 tails. Please push \"Show available tails...\" below to see the available tails.").arg(connectionManager.deviceModel.count);
                }
                else {
                    return qsTr("You are not currently connected to your tail, but we know of one tail. Push \"Connect\" to connect to it.");
                }
            ;
            }
        }
    }
    footer: Button {
        Layout.fillWidth: true; Layout.fillHeight: true;
        text: {
            if (connectionManager.discoveryRunning === false && connectionManager.deviceModel.count === 0) {
                return qsTr("Look for tails");
            }
            else if (connectionManager.deviceModel.count > 1) {
                return qsTr("Show available tails...");
            }
            else {
                return qsTr("Connect");
            }
        }
        visible: !(connectionManager.discoveryRunning === true && connectionManager.deviceModel.count === 0);
        onClicked: {
            if (connectionManager.discoveryRunning === false && connectionManager.deviceModel.count === 0) {
                connectionManager.startDiscovery();
            }
            else if(connectionManager.deviceModel.count === 1) {
                // Calling this will stop the discovery immediately and connect to the one tail that we've found
                connectionManager.stopDiscovery();
            }
            else {
                connectToTail.open();
            }
        }
    }
}