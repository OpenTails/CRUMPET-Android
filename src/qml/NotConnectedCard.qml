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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.AbstractCard {
    id: root;
    visible: opacity > 0
    opacity: (!Digitail.BTConnectionManager.isConnected || Digitail.BTConnectionManager.discoveryRunning) ? 1 : 0;
    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
    width: parent.width;
    Digitail.FilterProxyModel {
        id: deviceFilterProxy;
        sourceModel: Digitail.DeviceModel;
    }
    header: Kirigami.Heading {
        text: {
            if (!Digitail.PermissionsManager.hasBluetoothPermissions) {
                return i18nc("Header warning for missing scan permissions, for the gear connecting card", "Let us look for your gear");
            }
            else if (Digitail.BTConnectionManager.discoveryRunning === true) {
                return i18nc("Header for whilst scanning, for the gear connecting card", "Searching for gear...");
            }
            else {
                if (deviceFilterProxy.count === 0) {
                    return i18nc("Header warning that no gear has been found, for the gear connecting card", "No gear found");
                }
                else {
                    return i18ncp("Header notifying that gear has been found, for the gear connecting card", "One piece of gear available", "Multiple pieces of gear available", deviceFilterProxy.count);
                }
            }
        }
        level: 2
        padding: Kirigami.Units.smallSpacing;
        horizontalAlignment: Text.AlignHCenter;
        wrapMode: Text.Wrap;
        BusyIndicator {
            anchors {
                top: parent.top;
                right: parent.right;
            }
            height: parent.height;
            width: height;
            opacity: running;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            running: Digitail.BTConnectionManager.discoveryRunning;
        }
    }
    contentItem: Label {
        padding: Kirigami.Units.smallSpacing;
        wrapMode: Text.Wrap;
        horizontalAlignment: Text.AlignHCenter;
        text: {
            if (!Digitail.PermissionsManager.hasBluetoothPermissions) {
                if (Digitail.AppSettings.androidApiLevel > 30) {
                    return i18nc("Message warning for missing scan permissions, for the gear connecting card", "To be able to find your gear, we need you to grant permission to scan for nearby devices. Clicking the button below will show you a dialog that you need to press allow on.");
                }
                else {
                    return i18nc("Message warning for missing location permissions, for the gear connecting card", "To be able to find your gear, we need you to grant permission to access your location. Clicking the button below will show you a dialog that you need to press allow on. We do not use this information for anything else (feel free to get in touch if you want proof of this).");
                }
            }
            else if (Digitail.BTConnectionManager.discoveryRunning === true) {
                if (deviceFilterProxy.count === 0) {
                    return i18nc("Message whilst scanning warning that no gear has been found, for the gear connecting card", "None found yet...");
                }
                else {
                    return i18ncp("Message whilst scanning notifying that gear has been found, for the gear connecting card", "1 piece of gear found. Simply wait, or push \"Connect\" below to control it.", "Found %1 pieces of gear so far. To see them, push \"Show available gear...\" below.", deviceFilterProxy.count);
                }
            }
            else {
                if (deviceFilterProxy.count === 0) {
                    return i18nc("Message after scanning warning that no gear has been found, for the gear connecting card", "We were unable to find any gear. Please ensure that yours is nearby and switched on.");
                }
                else{
                    return i18ncp("Message after scanning notifying that gear has been found, for the gear connecting card", "You are not currently connected to your gear, but we know of one. Push \"Connect\" to connect to it.", "You are not currently connected to your gear, and we have found %1. Please push \"Show available gear...\" below to see what is available.", deviceFilterProxy.count);
                }
            }
        }
    }
    footer: ColumnLayout {
        Layout.fillWidth: true; Layout.fillHeight: true;
        Button {
            Layout.fillWidth: true; Layout.fillHeight: true;
            text: {
                if (!Digitail.PermissionsManager.hasBluetoothPermissions) {
                    if (Digitail.AppSettings.androidApiLevel > 30) {
                        return i18nc("Label for button for opening the settings tab to fix missing scan permissions, for the gear connecting card", "Get Scan Permission...");
                    }
                    else {
                        return i18nc("Label for button for opening the settings tab to fix missing location permissions, for the gear connecting card", "Get Location Permission...");
                    }
                } else if (deviceFilterProxy.count === 1) {
                    return i18nc("Label for button for connecting to a specific piece of gear, for the gear connecting card", "Connect to %1", deviceFilterProxy.data(deviceFilterProxy.index(0, 0), Digitail.DeviceModelTypes.Name))
                } else {
                    return i18nc("Label for button for showing a list of available gear, for the gear connecting card", "Show available gear...");
                }
            }
            visible: (!Digitail.PermissionsManager.hasBluetoothPermissions || deviceFilterProxy.count > 0)
            onClicked: {
                if (!Digitail.PermissionsManager.hasBluetoothPermissions) {
                    Digitail.PermissionsManager.requestBluetoothPermissions();
                }
                else if(deviceFilterProxy.count === 1) {
                    // Calling this will stop the discovery immediately and connect to the one tail that we've found
                    Digitail.BTConnectionManager.stopDiscovery();
                }
                else {
                    connectToTail.open();
                }
            }
        }
        Button {
            Layout.fillWidth: true; Layout.fillHeight: true;
            visible: !Digitail.BTConnectionManager.discoveryRunning && Digitail.PermissionsManager.hasBluetoothPermissions
            text: i18nc("Label for button which causes the list of available gear to be refreshed, for the gear connecting card", "Look for gear")
            onClicked: {
                Digitail.BTConnectionManager.startDiscovery();
            }
        }
    }
}
