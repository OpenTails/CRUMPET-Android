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

import org.kde.kirigami 2.4 as Kirigami
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
    title: qsTr("DIGITAiL");
    pageStack.defaultColumnWidth: root.width;

    Connections {
        target: BTConnectionManager;
        onMessage: {
            showPassiveNotification(message, 5000);
        }
        onDiscoveryRunningChanged: {
            if (BTConnectionManager.discoveryRunning === false) {
                console.log("device model count is " + BTConnectionManager.deviceCount + " and sheet is open: " + connectToTail.sheetOpen);
                if(BTConnectionManager.deviceCount === 1 && connectToTail.sheetOpen === false) {
                    // only one tail found? Well then, connect to that!
                    BTConnectionManager.connectToDevice(0);
                    connectingToTail.opacity = 1;
                }
            }
        }
        onIsConnectedChanged: {
            if (isConnected === true) {
                showPassiveNotification(qsTr("Connected to tail!"), 1000);
                if(root.pageToPush !== null) {
                    switchToPage(root.pageToPush);
                    root.pageToPush = null;
                }
            }
            connectingToTail.opacity = 0;
        }
    }
    Component.onCompleted: {
        switchToPage(welcomePage);
    }

    globalDrawer: Kirigami.GlobalDrawer {
        bannerImageSource: "qrc:/images/banner_image.png";
        // This is something of a hack... Can't access this properly as a property, so... this will have to do
        // Simply replacing the rectangle means we end up removing the handles and whatnot, so that's not cool
        Component.onCompleted: { background.color = "#3daee9"; }

        actions: [
            Kirigami.Action {
                text: qsTr("DIGITAiL");
                checked: pageStack.currentItem ? pageStack.currentItem.objectName === "welcomePage" : "";
                icon.name: ":/org/kde/kirigami/icons/go-home.svg";
                onTriggered: {
                    switchToPage(welcomePage);
                }
            },
            Kirigami.Action {
                text: qsTr("Alarm");
                checked: pageStack.currentItem ? (pageStack.currentItem.objectName === "alarmList" || pageStack.currentItem.objectName === "alarmListEditor"): "";
                icon.name: ":/org/kde/kirigami/icons/view-media-playlist.svg";
                onTriggered: {
                    if(!checked) {
                        switchToPage(alarmList);
                    }
                }
            },
            Kirigami.Action {
                text: qsTr("Move Lists");
                checked: pageStack.currentItem ? (pageStack.currentItem.objectName === "moveLists" || pageStack.currentItem.objectName === "moveListEditor"): "";
                icon.name: ":/org/kde/kirigami/icons/view-media-playlist.svg";
                onTriggered: {
                    if(!checked) {
                        switchToPage(moveLists);
                    }
                }
            },
            Kirigami.Action {
                text: qsTr("Tail Moves");
                checked: pageStack.currentItem ? pageStack.currentItem.objectName === "tailMoves" : "";
                icon.name: ":/images/tail_moves.svg";
                onTriggered: {
                    switchToPage(tailMoves);
                }
            },
            Kirigami.Action {
                text: qsTr("Glow Tips");
                checked: pageStack.currentItem ? pageStack.currentItem.objectName === "tailLights" : "";
                icon.name: ":/images/tail_lights.svg";
                onTriggered: {
                    switchToPage(tailLights);
                }
            },
            Kirigami.Action {
                text: qsTr("Casual Mode Settings");
                checked: pageStack.currentItem ? pageStack.currentItem.objectName === "idleMode" : "";
                icon.name: ":/org/kde/kirigami/icons/randomize.svg";
                visible: AppSettings.idleMode;
                onTriggered: {
                    switchToPage(idleModePage);
                }
            },
            Kirigami.Action {
            },
            Kirigami.Action {
                text: qsTr("Settings");
                icon.name: ":/org/kde/kirigami/icons/settings-configure.svg";

                onTriggered: {
                    showMessageBox(qsTr("Sorry"), qsTr("This page is coming soon..."));
                }
            },
            Kirigami.Action {
                text: qsTr("About");
                checked: pageStack.currentItem ? pageStack.currentItem.objectName === "aboutPage" : "";
                icon.name: ":/org/kde/kirigami/icons/help-about.svg";
                onTriggered: {
                    switchToPage(aboutPage);
                }
            }
        ]
    }
    Component {
        id: welcomePage;
        WelcomePage {}
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
        id: tailMoves;
        TailMoves {}
    }
    Component {
        id: tailLights;
        TailLights {}
    }
    Component {
        id: idleModePage;
        IdleModePage {}
    }
    Component {
        id: aboutPage;
        AboutPage {}
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
            text: qsTr("Connecting to tail...");
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
