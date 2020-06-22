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
import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0

Kirigami.AbstractCard {
    id: root;
    visible: opacity > 0
    opacity: BTConnectionManager.isConnected ? 0 : 1;
    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
    width: parent.width;
    FilterProxyModel {
        id: deviceFilterProxy;
        sourceModel: DeviceModel;
    }
    header: Kirigami.Heading {
        text: {
            if (BTConnectionManager.discoveryRunning === true) {
                return qsTr("Searching for gear...");
            }
            else {
                if (deviceFilterProxy.count === 0) {
                    return qsTr("No gear found");
                }
                else if (deviceFilterProxy.count === 1) {
                    return qsTr("One piece of gear available");
                }
                else {
                    return qsTr("Multiple pieces of gear available");
                }
            }
        }
        level: 2
        padding: Kirigami.Units.smallSpacing;
        horizontalAlignment: Text.AlignHCenter;
        BusyIndicator {
            anchors {
                top: parent.top;
                right: parent.right;
            }
            height: parent.height;
            width: height;
            opacity: running;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            running: BTConnectionManager.discoveryRunning;
        }
    }
    contentItem: Label {
        padding: Kirigami.Units.smallSpacing;
        wrapMode: Text.Wrap;
        horizontalAlignment: Text.AlignHCenter;
        text: {
            if (BTConnectionManager.discoveryRunning === true) {
                if (deviceFilterProxy.count === 0) {
                    return qsTr("None found yet...");
                }
                else if (deviceFilterProxy.count > 1) {
                    return qsTr("Found %1 pieces of gear so far. To see them, push \"Show available gear...\" below.").arg(deviceFilterProxy.count);
                }
                else {
                    return qsTr("1 piece of gear found. Simply wait, or push \"Connect\" below to control it.");
                }
            }
            else {
                if (deviceFilterProxy.count === 0) {
                    return qsTr("We were unable to find any gear. Please ensure that yours is nearby and switched on.");
                }
                else if (deviceFilterProxy.count > 1) {
                    return qsTr("You are not currently connected to your gear, and we have found %1. Please push \"Show available gear...\" below to see what is available.").arg(deviceFilterProxy.count);
                }
                else {
                    return qsTr("You are not currently connected to your gear, but we know of one. Push \"Connect\" to connect to it.");
                }
            ;
            }
        }
    }
    footer: Button {
        Layout.fillWidth: true; Layout.fillHeight: true;
        text: {
            if (BTConnectionManager.discoveryRunning === false && deviceFilterProxy.count === 0) {
                return qsTr("Look for gear");
            } else if (deviceFilterProxy.count === 1) {
                return qsTr("Connect to %1").arg(deviceFilterProxy.data(deviceFilterProxy.index(0, 0), 257))
            } else {
                return qsTr("Show available gear...");
            }
        }
        visible: !(BTConnectionManager.discoveryRunning === true && deviceFilterProxy.count === 0);
        onClicked: {
            if (BTConnectionManager.discoveryRunning === false && deviceFilterProxy.count === 0) {
                BTConnectionManager.startDiscovery();
            }
            else if(deviceFilterProxy.count === 1) {
                // Calling this will stop the discovery immediately and connect to the one tail that we've found
                BTConnectionManager.stopDiscovery();
            }
            else {
                connectToTail.open();
            }
        }
    }
}
