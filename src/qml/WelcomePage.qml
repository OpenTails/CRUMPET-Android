/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

Kirigami.Page {
    id: root;
    title: qsTr("Welcome");
    property QtObject connectionManager: null;
    actions {
        main: Kirigami.Action {
            text: connectionManager.isConnected ? "Disconnect" : "Connect";
            icon.name: connectionManager.isConnected ? ":/org/kde/kirigami/icons/network-disconnect.svg" : ":/org/kde/kirigami/icons/network-connect.svg";
            onTriggered: {
                if(connectionManager.isConnected) {
                    connectionManager.disconnectDevice();
                }
                else {
                    connectToTail.open();
                }
            }
        }
    }

    Column {
        width: root.width - Kirigami.Units.largeSpacing * 4;
        spacing: Kirigami.Units.largeSpacing;
        Kirigami.AbstractCard {
            opacity: connectionManager.isConnected ? 0 : 1;
            width: parent.width;
            header: Kirigami.Heading {
                text: qsTr("Not Connected");
                level: 2
            }
            contentItem: Label {
                wrapMode: Text.Wrap;
                text: qsTr("You are not currently connected to your tail...");
            }
        }
        GridLayout {
            id: commandLayout;
            width: parent.width;
            columns: root.width > root.height ? 3 : 2;
            columnSpacing: Kirigami.Units.largeSpacing;
            rowSpacing: Kirigami.Units.largeSpacing;
            Button {
                Layout.column: 0;
                Layout.row: 0;
                Layout.fillWidth: true; Layout.fillHeight: true;
                text: qsTr("Alarm");
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter;
                onClicked: {
                    connectToTail.open();
                }
            }
            Button {
                Layout.column: 0;
                Layout.row: 1;
                Layout.fillWidth: true; Layout.fillHeight: true;
                text: qsTr("Music");
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter;
                onClicked: {
                    connectToTail.open();
                }
            }
            Button {
                Layout.column: 1;
                Layout.row: 0;
                Layout.fillWidth: true; Layout.fillHeight: true;
                text: qsTr("My Moves");
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter;
                onClicked: {
                    connectToTail.open();
                }
            }
            Button {
                Layout.column: 1;
                Layout.row: 1;
                Layout.fillWidth: true; Layout.fillHeight: true;
                text: qsTr("Sound");
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter;
                onClicked: {
                    connectToTail.open();
                }
            }
            Button {
                text: connectionManager.isConnected ? qsTr("Tail Remote") : qsTr("Connect to your tail");
                Layout.column: commandLayout.columns === 2 ? 0 : 2;
                Layout.row: commandLayout.columns === 2 ? 2 : 0;
                Layout.columnSpan: 2;
                Layout.rowSpan: 2;
                Layout.fillWidth: true; Layout.fillHeight: true;
                onClicked: {
                    if(connectionManager.isConnected) {
                        pageStack.replace(tailMoves, {connectionManager: connectionManager});
                    }
                    else {
                        connectToTail.open();
                    }
                }
            }
        }
    }
}
