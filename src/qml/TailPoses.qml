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

Kirigami.ScrollablePage {
    id: root;
    title: qsTr("Tail Poses");
    property QtObject connectionManager: null;
    actions {
        main: Kirigami.Action {
            text: connectionManager.isConnected ? "Disconnect" : "Connect";
            icon.name: connectionManager.isConnected ? "network-disconnect" : "network-connect";
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
    Component {
        id: contactDelegate
        Item {
            width: poseGrid.cellWidth; height: poseGrid.cellHeight
            Rectangle {
                anchors {
                    fill: parent;
                    margins: Kirigami.Units.smallSpacing;
                }
                border {
                    width: (connectionManager.currentTailState === model.command) ? 1 : 0;
                    color: "silver";
                }
                radius: Kirigami.Units.smallSpacing;
            }
            Label {
                anchors.fill: parent;
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment: Text.AlignVCenter;
                text: model.text;
            }
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    connectionManager.currentTailState = model.command;
                }
            }
        }
    }
    ListModel {
        id: poseModel;
        ListElement {
            text: "Slow Wag 1";
            command: "S1"
        }
        ListElement {
            text: "Slow Wag 2";
            command: "S2"
        }
        ListElement {
            text: "Slow Wag 3";
            command: "S3"
        }
        ListElement {
            text: "Fast Wag";
            command: "FA"
        }
        ListElement {
            text: "Short Wag";
            command: "SH"
        }
        ListElement {
            text: "Happy Wag";
            command: "HA"
        }
        ListElement {
            text: "Erect";
            command: "ER"
        }
        ListElement {
            text: "Erect Pulse";
            command: "EP"
        }
        ListElement {
            text: "Tremble 1";
            command: "T1"
        }
        ListElement {
            text: "Tremble 2";
            command: "T2"
        }
        ListElement {
            text: "Erect Trem";
            command: "ET"
        }
    }
    GridView {
        id: poseGrid;
        currentIndex: 2;
        cellWidth: poseGrid.width / 3; cellHeight: cellWidth;
        model: poseModel;
        implicitWidth: Kirigami.Units.gridUnit * 30
        delegate: contactDelegate;
    }
}
