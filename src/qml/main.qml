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
    title: qsTr("Crumpet");
    pageStack.defaultColumnWidth: root.width;

    Connections {
        id: btConnection
        target: BTConnectionManager;
        onMessage: {
            showPassiveNotification(message, 5000);
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
                showPassiveNotification(qsTr("Connected successfully!"), 1000);
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
            if (BTConnectionManager.bluetoothState === 0 ) {
                showMessageBox(qsTr("Warning"), qsTr("Bluetooth is disabled"));
            } else if (BTConnectionManager.bluetoothState === 2) {
                showMessageBox(qsTr("Warning"), qsTr("No Bluetooth Device"));
            } else {
                console.log("Bluetooth is enabled");
            }
        }
    }

    Connections {
        target: AppSettings;

        onDeveloperModeChanged: {
            if (AppSettings.developerMode) {
                showMessageBox(qsTr("Developer mode"), qsTr("Developer mode is enabled"));
            } else {
                showMessageBox(qsTr("Developer mode"), qsTr("Developer mode is disabled"));
            }
        }

        onIdleModeTimeout: {
            showMessageBox(qsTr("Casual Mode is Off"), qsTr("Maybe its time for a rest... Let your tail or ears have a power-nap."));
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
            sourceModel: DeviceModel;
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
                text: qsTr("Crumpet");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "welcomePage";
                icon.name: "go-home";
                onTriggered: {
                    switchToPage(welcomePage);
                }
            },
            Kirigami.Action {
                text: qsTr("Alarm");
                checked: pageStack.currentItem && (pageStack.currentItem.objectName === "alarmList" || pageStack.currentItem.objectName === "alarmListEditor");
                icon.name: ":/images/alarm.svg";
                onTriggered: {
                    if(!checked) {
                        switchToPage(alarmList);
                    }
                }
            },
            Kirigami.Action {
                text: qsTr("Move Lists");
                checked: pageStack.currentItem && (pageStack.currentItem.objectName === "moveLists" || pageStack.currentItem.objectName === "moveListEditor");
                icon.name: ":/images/movelist.svg";
                onTriggered: {
                    if(!checked) {
                        switchToPage(moveLists);
                    }
                }
            },
            Kirigami.Action {
                text: qsTr("Moves");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "tailMoves";
                icon.name: ":/images/moves.svg";
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(tailMoves);
                }
            },
            Kirigami.Action {
                text: qsTr("Ear Poses");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "earPoses";
                icon.name: ":/images/earposes.svg";
                visible: hasListeningModel.count > 0;
                onTriggered: {
                    switchToPage(earPoses);
                }
            },
            Kirigami.Action {
                text: qsTr("Glow Tips");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "tailLights";
                icon.name: ":/images/glowtip.svg";
                visible: connectedDevicesModel.count > hasListeningModel.count;
                onTriggered: {
                    switchToPage(tailLights);
                }
            },
            Kirigami.Action {
                text: qsTr("Casual Mode Settings");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "idleMode";
                icon.name: ":/images/casualmode.svg";
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(idleModePage);
                }
            },
            Kirigami.Action {
                text: qsTr("Gear Gestures");
                checked: pageStack.currentItem && pageStack.currentItem.objectName === "gearGestures";
                icon.name: ":/images/movelist.svg";
                visible: connectedDevicesModel.count > 0;
                onTriggered: {
                    switchToPage(gearGestures);
                }
            },
            Kirigami.Action {
                text: qsTr("Developer Mode");
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
                text: qsTr("Settings");
                checked: pageStack.currentItem ? (pageStack.currentItem.objectName === "settingsPage"): "";
                icon.name: "settings-configure";

                onTriggered: {
                    if(!checked) {
                        switchToPage(settingsPage);
                    }
                }
            },
            Kirigami.Action {
                text: qsTr("About");
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

                showMessageBox(qsTr("Your gear is currently connected"),
                               qsTr("You are currently connected to some of your gear.\n\nAre you sure that you want to disconnect and quit?"),
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
                namePicker.pickName();
            }
        }

        property string deviceID: "0";

        description: qsTr("Enter a name to use for your new gear");
        placeholderText: qsTr("Enter the name here");
        buttonOkText: qsTr("Save");

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
            text: qsTr("Attempting to connect...");
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
    //                text: qsTr("Ok")
    //            }
    //        ]
    //    }

    MessageBox {
        id: messageBox;
    }
}
