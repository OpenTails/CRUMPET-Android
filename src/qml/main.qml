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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

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

    Connections {
        target: Digitail.PermissionsManager;
        ignoreUnknownSignals: true; // PermissionsManager isn't constructed before this card is first initialised, so we need to ignore that or end up with angry debug output
        function onPermissionsChanged() {
            btConnection.checkBluetoothState();
        }
    }

    Connections {
        id: btConnection
        target: Digitail.BTConnectionManager;
        function onMessage(message) {
            showPassiveNotification(message, 5000);
        }
        function onBlockingMessage(title, message) {
            messageBox.showMessageBox(title, message);
        }

        function onDiscoveryRunningChanged() {
            console.log("Discovery running changed to", Digitail.BTConnectionManager.discoveryRunning);
            if (Digitail.BTConnectionManager.discoveryRunning === false) {
                if(Digitail.BTConnectionManager.deviceCount === 1 && connectToTail.opened === false) {
                    // only one bit of gear found? Well then, connect to that!
                    Digitail.BTConnectionManager.connectToDevice("");
                    connectingToTail.connectingDevices += 1;
                }
            }
        }

        function onIsConnectedChanged(isConnected) {
            console.log("isConnected Changed to", isConnected);
            if (isConnected === true) {
                if(root.pageToPush !== null) {
                    switchToPage(root.pageToPush);
                    root.pageToPush = null;
                }
            }
        }

        function onDeviceConnected(deviceID) {
            console.log("Connected to new device with ID: " + deviceID, "and we are connecting to", connectingToTail.connectingDevices, "devices");
            // Only pop up the name picker if we're connecting to a single device, otherwise show a little
            // message after having connected the last thing to say what people can do to rename things,
            // without getting in the way too much
            if (connectingToTail.connectingMultipleDevices) {
                if (connectingToTail.connectingDevices === 1) {
                    showPassiveNotification(i18nc("Text for the notification upon successfully connecting to the final device, after connecting to multiple devices simultaneously, informing the user that it has happened, and how they can rename the devices", "Connected successfully!\nRename gear by tapping their name"), 2000);
                }
            } else {
                showPassiveNotification(i18nc("Text for the notification upon connecting successfully to a device", "Connected successfully!"), 1000);
                let deviceIsKnown = false;
                for(let i = 0; i < Digitail.BTConnectionManager.deviceCount; ++i) {
                    let maybeNeedle = Digitail.DeviceModel.data(Digitail.DeviceModel.index(i, 0), Digitail.DeviceModelTypes.DeviceID);
                    if (maybeNeedle == namePicker.deviceID) {
                        deviceIsKnown = Digitail.DeviceModel.data(Digitail.DeviceModel.index(i, 0), Digitail.DeviceModelTypes.IsKnown);
                        break;
                    }
                }
                if (deviceIsKnown === false) {
                    namePicker.checkDeviceName(deviceID);
                    Digitail.BTConnectionManager.setDeviceProperty(deviceID, "isKnown", true);
                }
            }
            connectingToTail.connectingDevices -= 1;
        }

        function onBluetoothStateChanged() {
            checkBluetoothState();
        }

        function checkBluetoothState() {
            if (Digitail.PermissionsManager.hasBluetoothPermissions) {
                if (Digitail.BTConnectionManager.bluetoothState === -1) {
                    showMessageBox(i18nc("Title for the warning for having Bluetooth disabled", "Warning"), i18nc("Message for the warning for having Bluetooth disabled", "Bluetooth is disabled"));
                } else if (Digitail.BTConnectionManager.bluetoothState === -2) {
                    showMessageBox(i18nc("Title for the warning for not having detected any Bluetooth devices", "Warning"), i18nc("Message for the warning for not having detected any Bluetooth devices", "No Bluetooth Device"));
                } else {
                    console.log("Bluetooth is enabled, state is", Digitail.BTConnectionManager.bluetoothState);
                }
            }
        }
    }

    Connections {
        target: Digitail.AppSettings;

        function onDeveloperModeChanged() {
            if (Digitail.AppSettings.developerMode) {
                showMessageBox(i18nc("Title for the popup for having enabled Developer Mode", "Developer mode"), i18nc("Message for the popup for having enabled Developer Mode", "Developer mode is enabled"));
            } else {
                showMessageBox(i18nc("Title for the popup for having disabled Developer Mode", "Developer mode"), i18nc("Message for the popup for having disabled Developer Mode", "Developer mode is disabled"));
            }
        }

        function onIdleModeTimeout() {
            showMessageBox(i18nc("Title for the popup for getting a timeout for Casual Mode", "Casual Mode is Off"), i18nc("Message for the popup for getting a timeout for Casual Mode", "Maybe its time for a rest... Let your tail or ears have a power-nap."));
        }
    }

    Component.onCompleted: {
        switchToPage(welcomePage);
        btConnection.checkBluetoothState();
    }

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: globalDrawer

        // bannerImageSource: "qrc:/images/banner_image.png";
        // This is something of a hack... Can't access this properly as a property, so... this will have to do
        // Simply replacing the rectangle means we end up removing the handles and whatnot, so that's not cool
//         Component.onCompleted: { background.color = "#3daee9"; }

        Digitail.FilterProxyModel {
            id: connectedDevicesModel
            // This is a bit of a hack. Without this, the DeviceModel is set before
            // the replication from the service is completed, which ends up with us
            // just outright having no information to work with. So, workaround.
            sourceModel: Digitail.BTConnectionManager.isConnected ? Digitail.DeviceModel : null;
            filterRole: Digitail.DeviceModelTypes.IsConnected;
            filterBoolean: true;
        }
        Digitail.FilterProxyModel {
            id: hasListeningModel;
            sourceModel: connectedDevicesModel;
            filterRole: Digitail.DeviceModelTypes.HasListening;
            filterBoolean: true;
        }

        actions: [
            Kirigami.Action {
                text: i18nc("Button for opening the Crumpet page, from the landing page", "Crumpet");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "welcomePage";
                icon.name: "go-home";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                onTriggered: {
                    switchToPage(welcomePage);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Alarm page, from the landing page", "Alarm");
                checked: pageStack.currentItem && (pageStack.currentItem.objectName === "alarmList" || pageStack.currentItem.objectName === "alarmListEditor");
                icon.name: "qrc:/images/alarm.svg";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                onTriggered: {
                    if(!checked) {
                        switchToPage(alarmList);
                    }
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Move List page, from the landing page", "Move Lists");
                checked: pageStack.currentItem && (pageStack.currentItem.objectName === "moveLists" || pageStack.currentItem.objectName === "moveListEditor");
                icon.name: "qrc:/images/movelist.svg";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                onTriggered: {
                    if(!checked) {
                        switchToPage(moveLists);
                    }
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Tail Gear Moves page, from the landing page", "Moves");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "tailMoves";
                icon.name: "qrc:/images/moves.svg";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(tailMoves);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the EarGear Poses page, from the landing page", "Ear Poses");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "earPoses";
                icon.name: "qrc:/images/earposes.svg";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                visible: hasListeningModel.count > 0;
                onTriggered: {
                    switchToPage(earPoses);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Glow Tips page, from the landing page", "Glow Tips");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "tailLights";
                icon.name: "qrc:/images/glowtip.svg";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                visible: connectedDevicesModel.count > hasListeningModel.count;
                onTriggered: {
                    switchToPage(tailLights);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the page for setting up the Casual Mode, from the landing page", "Casual Mode Settings");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "idleMode";
                icon.name: "qrc:/images/casualmode.svg";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(idleModePage);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Gear Gestures page, from the landing page", "Gear Gestures");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "gearGestures";
                icon.name: "qrc:/images/movelist.svg";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(gearGestures);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the Developer Mode page, from the landing page", "Developer Mode");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "developerModePage";
                icon.name: "code-context";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                visible: Digitail.AppSettings !== null ? Digitail.AppSettings.developerMode : false;

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
                displayHint: Kirigami.DisplayHint.KeepVisible;

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
                displayHint: Kirigami.DisplayHint.KeepVisible;
                onTriggered: {
                    switchToPage(aboutPage);
                }
            },
            Kirigami.Action {
                text: i18nc("Button for opening the TailCo website in a browser, from the landing page", "TheTailCompany.com");
                checked: false;
                icon.name: "internet-services";
                displayHint: Kirigami.DisplayHint.KeepVisible;
                onTriggered: {
                    Qt.openUrlExternally("https://thetailcompany.com/");
                }
            }
        ]
        content: ColumnLayout {
            spacing: 0
            Layout.fillHeight: true
            Timer {
                property int clicksCount: 0
                id: clicksTimer;
                interval: 1500;

                onTriggered: {
                    if (clicksTimer.clicksCount >= 5) {
                        globalDrawer.close();
                        Digitail.AppSettings.developerMode = !Digitail.AppSettings.developerMode;
                    } else {
                        clicksTimer.clicksCount = 0;
                    }
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            Image {
                Layout.fillWidth: true
                Layout.leftMargin: -globalDrawer.leftPadding
                Layout.rightMargin: -globalDrawer.rightPadding
                Layout.bottomMargin: -globalDrawer.bottomPadding
                source: "qrc:/images/banner_image.png";
                TapHandler {
                    onTapped: {
                        if (!clicksTimer.running) {
                            clicksTimer.clicksCount = 0;
                            clicksTimer.start();
                        }
                        clicksTimer.clicksCount++;
                    }
                }
            }
        }
    }
    Component {
        id: welcomePage;

        WelcomePage {
            onBackRequested: {
                if (!Digitail.BTConnectionManager.isConnected) {
                    return;
                }

                event.accepted = true;

                showMessageBox(i18nc("Title for the confirmation popup for disconnecting your gear", "Your gear is currently connected"),
                               i18nc("Message for the confirmation popup for disconnecting your gear", "You are currently connected to some of your gear.\n\nAre you sure that you want to disconnect and quit?"),
                               function () {
                                   if(Digitail.BTConnectionManager.isConnected) {
                                       Digitail.BTConnectionManager.disconnectDevice("");
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
        id: listenSettings;
        ListenSettings {}
    }
    Component {
        id: tiltSettings;
        TiltSettings {}
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
        width: root.width - Kirigami.Units.largeSpacing * 2

        function checkDeviceName(deviceID, forcePicking = false) {
            let deviceIsKnown = false;
            for(let i = 0; i < Digitail.BTConnectionManager.deviceCount; ++i) {
                let maybeNeedle = Digitail.DeviceModel.data(Digitail.DeviceModel.index(i, 0), Digitail.DeviceModelTypes.DeviceID);
                if (maybeNeedle == deviceID) {
                    deviceIsKnown = Digitail.DeviceModel.data(Digitail.DeviceModel.index(i, 0), Digitail.DeviceModelTypes.IsKnown);
                    namePicker.previousName = Digitail.DeviceModel.data(Digitail.DeviceModel.index(i, 0), Digitail.DeviceModelTypes.Name);
                    break;
                }
            }
            if (deviceID && (forcePicking || deviceIsKnown === false)) {
                namePicker.deviceID = deviceID;
                namePicker.pickName();
            }
        }

        property string deviceID: "0";
        property string previousName: ""

        description: i18nc("Description for the prompt for entering a name for your Gear", "Enter a name to use for your gear");
        placeholderText: i18nc("Placeholder text for the prompt for entering a name for your Gear", "Enter the new name for %1 here", namePicker.previousName === "" ? "(unnamed)" : namePicker.previousName);
        buttonOkText: i18nc("Button for confirming the save of your new name, for the prompt for entering a name for your Gear", "Save");

        onNamePicked: function(name) {
            Digitail.BTConnectionManager.setDeviceName(namePicker.deviceID, name);
            namePicker.close();
        }
    }

    ConnectToTail {
        id: connectToTail;
        onAttemptToConnect: function(deviceID) {
            root.pageToPush = connectToTail.pageToPush;
            Digitail.BTConnectionManager.connectToDevice(deviceID);
            connectingToTail.connectingDevices += 1;
        }
    }
    DisconnectOptions {
        id: disconnectionOptions;
        width: root.width - Kirigami.Units.largeSpacing * 2
    }

    QtObject {
        id: connectingToTail;
        property int connectingDevices: 0
        property bool connectingMultipleDevices: false
        onConnectingDevicesChanged: {
            if (connectingDevices > 1) {
                connectingMultipleDevices = true;
            } else if (connectingDevices === 0) {
                connectingMultipleDevices = false;
            }
        }
    }

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
                model: Digitail.FilterProxyModel {
                    sourceModel: Digitail.DeviceModel
                    filterRole: Digitail.DeviceModelTypes.OperationInProgress;
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
