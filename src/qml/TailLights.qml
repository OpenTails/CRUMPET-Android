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
    title: qsTr("Lights");
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
                    width: model.isRunning ? 1 : 0;
                    color: "silver";
                }
                radius: Kirigami.Units.smallSpacing;
            }
            Label {
                anchors.fill: parent;
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment: Text.AlignVCenter;
                text: model.name;
            }
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                    connectionManager.runCommand(model.command);
                }
            }
        }
    }
    GridView {
        id: poseGrid;
        currentIndex: 2;
        property int cellCount: root.width > root.height ? 6 : 3;
        cellWidth: poseGrid.width / cellCount; cellHeight: cellWidth;
        model: connectionManager ? connectionManager.commandModel : null;
        implicitWidth: Kirigami.Units.gridUnit * 30
        delegate: contactDelegate;
    }
}
