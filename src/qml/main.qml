/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0

Kirigami.ApplicationWindow {
    id: root;

    property QtObject pageToPush: null;

    function switchToPage(pageToSwitchTo) {
        if (pageToSwitchTo === welcomePage) {
            pageStack.clear();
            pageStack.push(pageToSwitchTo);
        }
        else if (pageStack.depth === 1) {
            pageStack.push(pageToSwitchTo);
        }
        else {
            pageStack.pop();
            pageStack.push(pageToSwitchTo);
        }
    }

    // Now it shows just at the top, so to use it widely we should setup InlineMessage more correctly
    //    function showWarningInlineMessage(text) {
    //        showInlineMessage(text, Kirigami.MessageType.Warning)
    //    }

    //    // Now it shows just at the top, so to use it widely we should setup InlineMessage more correctly
    //    function showInlineMessage(text, type) {
    //        inlineMessageWithOkButton.type = type
    //        inlineMessageWithOkButton.text = text
    //        inlineMessageWithOkButton.visible = true
    //    }

    function showMessageBox(header, text, okHandler) {
        messageBox.showMessageBox(header, text, okHandler)
    }

    visible: true;
    title: i18nc("Title of the application", "Crumpet");
    pageStack.defaultColumnWidth: root.width;

    Connections {
        target: PermissionsManager;
        ignoreUnknownSignals: true; // PermissionsManager isn't constructed before this card is first initialised, so we need to ignore that or end up with angry debug output
        function onPermissionsChanged() {
            root.hasScanPermission = PermissionsManager.hasPermission(BTConnectionManager.bluetoothScanPermissionName);
            root.hasConnectPermission = BTConnectionManager.bluetoothConnectPermissionName === "" ? true : PermissionsManager.hasPermission(BTConnectionManager.bluetoothConnectPermissionName);
        }
    }
    property bool hasScanPermission: PermissionsManager.hasPermission(BTConnectionManager.bluetoothScanPermissionName);
    property bool hasConnectPermission: BTConnectionManager.bluetoothConnectPermissionName === "" ? true : PermissionsManager.hasPermission(BTConnectionManager.bluetoothConnectPermissionName);
    onHasScanPermissionChanged: btConnection.checkBluetoothState();

    Connections {
        id: btConnection
        target: BTConnectionManager;
        onMessage: {
            showPassiveNotification(message, 5000);
        }
        onBlockingMessage: {
            messageBox.showMessageBox(title, message);
        }

        onDiscoveryRunningChanged: {
            if (BTConnectionManager.discoveryRunning === false) {
                console.log("device model count is " + BTConnectionManager.deviceCount + " and sheet is open: " + connectToTail.sheetOpen);
                if(BTConnectionManager.deviceCount === 1 && connectToTail.sheetOpen === false) {
                    // only one tail found? Well then, connect to that!
                    BTConnectionManager.connectToDevice("");
                    connectingToTail.opacity = 1;
                }
            }
        }

        onIsConnectedChanged: {
            if (isConnected === true) {
                showPassiveNotification(i18nc("Text for the notification upon connecting successfully to a device", "Connected successfully!"), 1000);
                if(root.pageToPush !== null) {
                    switchToPage(root.pageToPush);
                    root.pageToPush = null;
                }
            }
            connectingToTail.opacity = 0;
        }

        onDeviceConnected: {
            console.debug("Connected to new device with ID: " + deviceID);
            namePicker.checkDeviceName(deviceID);
        }

        onBluetoothStateChanged: {
            checkBluetoothState();
        }

        function checkBluetoothState() {
            if (root.hasScanPermission) {
                if (BTConnectionManager.bluetoothState === 0 ) {
                    showMessageBox(i18nc("Title for the warning for having Bluetooth disabled", "Warning"), i18nc("Message for the warning for having Bluetooth disabled", "Bluetooth is disabled"));
                } else if (BTConnectionManager.bluetoothState === 2) {
                    showMessageBox(i18nc("Title for the warning for not having detected any Bluetooth devices", "Warning"), i18nc("Message for the warning for not having detected any Bluetooth devices", "No Bluetooth Device"));
                } else {
                    console.log("Bluetooth is enabled");
                }
            }
        }
    }

    Connections {
        target: AppSettings;

        onDeveloperModeChanged: {
            if (AppSettings.developerMode) {
                showMessageBox(i18nc("Title for the popup for having enabled Developer Mode", "Developer mode"), i18nc("Message for the popup for having enabled Developer Mode", "Developer mode is enabled"));
            } else {
                showMessageBox(i18nc("Title for the popup for having disabled Developer Mode", "Developer mode"), i18nc("Message for the popup for having disabled Developer Mode", "Developer mode is disabled"));
            }
        }

        onIdleModeTimeout: {
            showMessageBox(i18nc("Title for the popup for getting a timeout for Casual Mode", "Casual Mode is Off"), i18nc("Message for the popup for getting a timeout for Casual Mode", "Maybe its time for a rest... Let your tail or ears have a power-nap."));
        }
    }

    Component.onCompleted: {
        switchToPage(welcomePage);
        btConnection.checkBluetoothState();
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: globalDrawer

        property int clicksCount: 0

        bannerVisible: true;
        bannerImageSource: "qrc:/images/banner_image.png";
        // This is something of a hack... Can't access this properly as a property, so... this will have to do
        // Simply replacing the rectangle means we end up removing the handles and whatnot, so that's not cool
//         Component.onCompleted: { background.color = "#3daee9"; }

        onBannerClicked: {
            if (!clicksTimer.running) {
                clicksCount = 0;
                clicksTimer.start();
            }

            clicksCount++;
        }

        FilterProxyModel {
            id: connectedDevicesModel
            // This is a bit of a hack. Without this, the DeviceModel is set before
            // the replication from the service is completed, which ends up with us
            // just outright having no information to work with. So, workaround.
            sourceModel: BTConnectionManager.isConnected ? DeviceModel : null;
            filterRole: 262; // the isConnected role
            filterBoolean: true;
        }
        FilterProxyModel {
            id: hasListeningModel;
            sourceModel: connectedDevicesModel;
            filterRole: 265; // the hasListening role
            filterBoolean: true;
        }

        Timer {
            id: clicksTimer;
            interval: 1500;

            onTriggered: {
                if (globalDrawer.clicksCount >= 5) {
                    globalDrawer.close();
                    AppSettings.developerMode = !AppSettings.developerMode;
                } else {
                    globalDrawer.clicksCount = 0;
                }
            }
        }

        actions: [
            Kirigami.Action {
                text: i18nc("Button for opening the Crumpet page, from the landing page", "Crumpet");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "welcomePage";
                icon.name: "go-home";
                onTriggered: {
                    switchToPage(welcomePage);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Alarm page, from the landing page", "Alarm");
                checked: pageStack.currentItem && (pageStack.currentItem.objectName === "alarmList" || pageStack.currentItem.objectName === "alarmListEditor");
                icon.name: ":/images/alarm.svg";
                onTriggered: {
                    if(!checked) {
                        switchToPage(alarmList);
                    }
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Move List page, from the landing page", "Move Lists");
                checked: pageStack.currentItem && (pageStack.currentItem.objectName === "moveLists" || pageStack.currentItem.objectName === "moveListEditor");
                icon.name: ":/images/movelist.svg";
                onTriggered: {
                    if(!checked) {
                        switchToPage(moveLists);
                    }
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Tail Gear Moves page, from the landing page", "Moves");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "tailMoves";
                icon.name: ":/images/moves.svg";
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(tailMoves);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the EarGear Poses page, from the landing page", "Ear Poses");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "earPoses";
                icon.name: ":/images/earposes.svg";
                visible: hasListeningModel.count > 0;
                onTriggered: {
                    switchToPage(earPoses);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Glow Tips page, from the landing page", "Glow Tips");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "tailLights";
                icon.name: ":/images/glowtip.svg";
                visible: connectedDevicesModel.count > hasListeningModel.count;
                onTriggered: {
                    switchToPage(tailLights);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the page for setting up the Casual Mode, from the landing page", "Casual Mode Settings");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "idleMode";
                icon.name: ":/images/casualmode.svg";
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(idleModePage);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Gear Gestures page, from the landing page", "Gear Gestures");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "gearGestures";
                icon.name: ":/images/movelist.svg";
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(gearGestures);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Developer Mode page, from the landing page", "Developer Mode");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "developerModePage";
                icon.name: "code-context";
                visible: AppSettings !== null ? AppSettings.developerMode : false;

                onTriggered: {
                    if(!checked) {
                        switchToPage(developerModePage);
                    }
                }
            },
            Kirigami.Action {
                separator: true;
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Settings Page, from the landing page", "Settings");
                checked: pageStack.currentItem ? (pageStack.currentItem.objectName === "settingsPage"): "";
                icon.name: "settings-configure";

                onTriggered: {
                    if(!checked) {
                        switchToPage(settingsPage);
                    }
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the About Us page, from the landing page", "About");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "aboutPage";
                icon.name: "help-about";
                onTriggered: {
                    switchToPage(aboutPage);
                }
            }
        ]
    }
    Component {
        id: welcomePage;

        WelcomePage {
            onBackRequested: {
                if (!BTConnectionManager.isConnected) {
                    return;
                }

                event.accepted = true;

                showMessageBox(i18nc("Title for the confirmation popup for disconnecting your gear", "Your gear is currently connected"),
                               i18nc("Message for the confirmation popup for disconnecting your gear", "You are currently connected to some of your gear.\n\nAre you sure that you want to disconnect and quit?"),
                               function () {
                                   if(BTConnectionManager.isConnected) {
                                       BTConnectionManager.disconnectDevice();
                                   }

                                   Qt.quit();
                               });
            }
        }
    }
    Component {
        id: alarmList;
        AlarmList {}
    }
    Component {
        id: moveLists;
        MoveLists {}
    }
    Component {
        id: gearGestures;
        GearGestures {}
    }
    Component {
        id: tailMoves;
        TailMoves {}
    }
    Component {
        id: tailLights;
        TailLights {}
    }
    Component {
        id: earPoses;
        EarPoses {}
    }
    Component {
        id: idleModePage;
        IdleModePage {}
    }
    Component {
        id: settingsPage;
        SettingsPage {}
    }
    Component {
        id: developerModePage;
        DeveloperModePage {}
    }
    Component {
        id: aboutPage;
        AboutPage {}
    }

    NamePicker {
        id: namePicker;

        function checkDeviceName(deviceID) {
            if (deviceID && !AppSettings.deviceNames[deviceID]) {
                namePicker.deviceID = deviceID;
                for(var i = 0; i < BTConnectionManager.deviceCount; ++i) {
                    var deviceID = DeviceModel.data(DeviceModel.index(i, 0), 258); // DeviceID role
                    if (deviceID == namePicker.deviceID) {
                        namePicker.previousName = DeviceModel.data(DeviceModel.index(i, 0), 257); // Name role
                        break;
                    }
                }
                namePicker.pickName();
            }
        }

        property string deviceID: "0";
        property string previousName: ""

        description: i18nc("Description for the prompt for entering a name for your Gear", "Enter a name to use for your new gear");
        placeholderText: i18nc("Placeholder text for the prompt for entering a name for your Gear", "Enter the new name for %1 here", namePicker.previousName === "" ? "(unnamed)" : namePicker.previousName);
        buttonOkText: i18nc("Button for confirming the save of your new name, for the prompt for entering a name for your Gear", "Save");

        onNamePicked: {
            BTConnectionManager.setDeviceName(deviceID, name);
            namePicker.close();
        }
    }

    ConnectToTail {
        id: connectToTail;
        onAttemptToConnect: {
            root.pageToPush = pageToPush;
            BTConnectionManager.connectToDevice(deviceID);
            connectingToTail.opacity = 1;
        }
    }
    DisconnectOptions {
        id: disconnectionOptions;
    }

    Item {
        id: connectingToTail;
        anchors.fill: parent;
        opacity: 0;
        Behavior on opacity { PropertyAnimation { duration: 250; } }
        Rectangle {
            color: "black";
            opacity: 0.2;
            anchors.fill: parent;
        }
        MouseArea {
            anchors.fill: parent;
            enabled: connectingToTail.opacity > 0;
            onClicked: {}
        }
        Label {
            anchors {
                bottom: parent.verticalCenter;
                bottomMargin: Kirigami.Units.smallMargin;
                horizontalCenter: parent.horizontalCenter;
            }
            text: i18nc("Label for your gear that gets shown when attempting to connect to it", "Attempting to connect...");
        }
        BusyIndicator {
            anchors {
                top: parent.verticalCenter;
                topMargin: Kirigami.Units.smallMargin;
                horizontalCenter: parent.horizontalCenter;
            }
            running: connectingToTail.opacity > 0;
        }
    }

    // Now it shows just at the top, so to use it widely we should setup InlineMessage more correctly
    //    Kirigami.InlineMessage {
    //        id: inlineMessageWithOkButton

    //        actions: [
    //            Kirigami.Action {
    //                text: i18nc("Confirmation button for an inline message", "Ok")
    //            }
    //        ]
    //    }

    MessageBox {
        id: messageBox;
    }

    Item {
        anchors.fill: parent;
        visible: opacity > 0
        opacity: deviceProgressRepeater.count > 0 ? 1 : 0
        Behavior on opacity { PropertyAnimation { duration: 250; } }
        Rectangle {
            anchors.fill: parent;
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            opacity: 0.7
        }
        MouseArea {
            anchors.fill: parent;
            onClicked: { /* Do nothing, we're just blocking clicks */ }
        }
        ColumnLayout {
            anchors {
                fill: parent;
                margins: Kirigami.Units.largeSpacing;
            }
            Item { Layout.fillWidth: true; Layout.fillHeight: true; }
            Repeater {
                id: deviceProgressRepeater
                model: FilterProxyModel {
                    sourceModel: DeviceModel
                    filterRole: 274; // the operationInProgress role
                    filterBoolean: true;
                }
                delegate: Kirigami.AbstractCard {
                    Layout.fillWidth: true;
                    header: Kirigami.Heading {
                        text: model.name
                        elide: Text.ElideRight
                    }
                    contentItem: Label {
                        text: model.progressDescription;
                        wrapMode: Text.Wrap;
                    }
                    footer: ProgressBar {
                        from: 1
                        to: 100
                        indeterminate: model.deviceProgress === 0
                        value: model.deviceProgress
                    }
                }
            }
            Item { Layout.fillWidth: true; Layout.fillHeight: true; }
        }
    }
}
