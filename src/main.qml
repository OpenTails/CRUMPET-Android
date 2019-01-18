/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import "qml"

Kirigami.ApplicationWindow {
    id: root;
    visible: true;
    title: qsTr("DIGITAiL");

    function switchToPage(pageToSwitchTo) {
        if (pageToSwitchTo === welcomePage) {
            pageStack.clear();
            pageStack.push(pageToSwitchTo, {connectionManager: connectionManager});
        }
        else if (pageStack.depth === 1) {
            pageStack.push(pageToSwitchTo, {connectionManager: connectionManager});
        }
        else {
            pageStack.pop();
            pageStack.push(pageToSwitchTo, {connectionManager: connectionManager});
        }
    }

    BTConnectionManager {
        id: connectionManager;
        onMessage: {
            showPassiveNotification(message, 5000);
        }
        property QtObject pageToPush: null;
        onDiscoveryRunningChanged: {
            if (discoveryRunning === false) {
                if(connectionManager.deviceModel.count === 1 && connectToTail.sheetOpen === false) {
                    // only one tail found? Well then, connect to that!
                    connectToDevice(deviceModel.getDeviceID(0));
                    connectingToTail.opacity = 1;
                }
            }
        }
        onIsConnectedChanged: {
            if (isConnected === true) {
                showPassiveNotification(qsTr("Connected to tail!"), 1000);
                if(pageToPush !== null) {
                    switchToPage(pageToPush);
                    pageToPush = null;
                }
            }
            connectingToTail.opacity = 0;
        }
    }
    Component.onCompleted: {
        switchToPage(welcomePage);
    }

    /**
    * Example from MDN
    * Returns a random integer between min (inclusive) and max (inclusive).
    * The value is no lower than min (or the next integer greater than min
    * if min isn't an integer) and no greater than max (or the next integer
    * lower than max if max isn't an integer).
    * Using Math.round() will give you a non-uniform distribution!
    */
    function getRandomInt(min, max) {
        min = Math.ceil(min);
        max = Math.floor(max);
        return Math.floor(Math.random() * (max - min + 1)) + min;
    }
    function idleModePush() {
        if(connectionManager.commandQueue.count === 0 && AppSettings.idleMode === true && AppSettings.idleCategories.length > 0) {
            connectionManager.commandQueue.pushCommand(connectionManager.commandModel.getRandomCommand(AppSettings.idleCategories));
            connectionManager.commandQueue.pushPause(getRandomInt(AppSettings.idleMinPause, AppSettings.idleMaxPause) * 1000);
        }
    }
    Connections {
        target: connectionManager.commandQueue;
        onCountChanged: idleModePush();
    }
    Connections {
        target: AppSettings;
        onIdleModeChanged: idleModePush();
        onIdleCategoriesChanged: idleModePush();
    }

    globalDrawer: Kirigami.GlobalDrawer {
        bannerImageSource: "qrc:/images/bannerimage.png";
        // This is something of a hack... Can't access this properly as a property, so... this will have to do
        // Simply replacing the rectangle means we end up removing the handles and whatnot, so that's not cool
        Component.onCompleted: { background.color = "#3daee9"; }
        topContent: Item {
            visible: height > 0;
            height: connectionManager.isConnected === true ? batteryLabel.height : 0;
            Layout.fillWidth: true;
            Label {
                id: batteryLabel;
                text: "Tail battery:"
                height: Kirigami.Units.iconSizes.small;
            }
            Row {
                anchors.right: parent.right;
                Repeater {
                    model: 4;
                    Rectangle {
                        height: Kirigami.Units.iconSizes.small;
                        width: height;
                        radius: height / 2;
                        border {
                            width: 1;
                            color: "black";
                        }
                        color: "transparent";
                        anchors.verticalCenter: parent.verticalCenter;
                        Rectangle {
                            anchors.centerIn: parent;
                            radius: height / 2;
                            height: parent.height - Kirigami.Units.smallSpacing * 2;
                            width: height;
                            color: "black";
                            visible: modelData < connectionManager.batteryLevel;
                        }
                    }
                }
            }
        }
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
            },
            Kirigami.Action {
                text: qsTr("Tail Moves");
                checked: pageStack.currentItem ? pageStack.currentItem.objectName === "tailMoves" : "";
                icon.name: ":/images/tail-moves.svg";
                onTriggered: {
                    switchToPage(tailMoves);
                }
            },
            Kirigami.Action {
                text: qsTr("Tail Lights");
                checked: pageStack.currentItem ? pageStack.currentItem.objectName === "tailLights" : "";
                icon.name: ":/images/tail-lights.svg";
                onTriggered: {
                    switchToPage(tailLights);
                }
            },
            Kirigami.Action {
                text: qsTr("Idle Mode Settings");
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
                text: qsTr("About");
                checked: pageStack.currentItem ? pageStack.currentItem.objectName === "aboutPage" : "";
                icon.name: ":/org/kde/kirigami/icons/help-about.svg";
                onTriggered: {
                    switchToPage(aboutPage);
                }
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }
    Component {
        id: welcomePage;
        WelcomePage {}
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
        connectionManager: connectionManager;
        onAttemptToConnect: {
            connectionManager.pageToPush = pageToPush;
            connectionManager.connectToDevice(deviceID);
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
}
