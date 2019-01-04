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

    BTConnectionManager {
        id: connectionManager;
        onMessage: {
            showPassiveNotification(message, 5000);
        }
        onIsConnectedChanged: {
            if (isConnected === true) {
                showPassiveNotification(qsTr("Connected to tail!"), 1000);
                pageStack.replace(tailMoves, {connectionManager: connectionManager});
            }
            connectingToTail.opacity = 0;
        }
    }
    Component.onCompleted: {
        pageStack.replace(welcomePage, {connectionManager: connectionManager});
    }

    globalDrawer: Kirigami.GlobalDrawer {
        title: "DIGITAiL";
        titleIcon: "://logo.svg";
        actions: [
            Kirigami.Action {
                text: qsTr("Welcome");
                onTriggered: {
                    pageStack.replace(welcomePage, {connectionManager: connectionManager});
                }
            },
            Kirigami.Action {
                text: qsTr("Tail Moves");
                onTriggered: {
                    pageStack.replace(tailMoves, {connectionManager: connectionManager});
                }
            },
            Kirigami.Action {
                text: qsTr("About");
                onTriggered: {
                    pageStack.replace(aboutPage);
                }
            }
        ]
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
        id: aboutPage;
        AboutPage {}
    }
    ConnectToTail {
        id: connectToTail;
        connectionManager: connectionManager;
        onAttemptToConnect: {
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
