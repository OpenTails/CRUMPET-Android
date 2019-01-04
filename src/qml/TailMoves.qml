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

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.5 as Kirigami
import org.thetailcompany.digitail 1.0

Kirigami.ScrollablePage {
    id: root;
    title: qsTr("Tail Moves");
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
        id: categoryDelegate;
        Kirigami.AbstractCard {
            Layout.fillHeight: true
            width: Kirigami.Units.gridUnit * 30;
            header: Kirigami.Heading {
                text: model.name
                level: 2
            }
            contentItem: Item {
                implicitHeight: commandGrid.cellHeight * Math.ceil(commandGrid.count / 3);
                FilterProxyModel {
                    id: filterProxy;
                    sourceModel: connectionManager.commandModel;
                    filterRole: 260; // the Category role ID
                    filterString: model.category;
                }
                Component {
                    id: commandDelegate
                    Item {
                        width: commandGrid.cellWidth; height: commandGrid.cellHeight
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
                            wrapMode: Text.Wrap;
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
                    id: commandGrid;
                    cellWidth: commandGrid.width / 3;
                    cellHeight: cellWidth;
                    anchors.fill: parent;
                    model: filterProxy;
                    delegate: commandDelegate;
                }
            }
        }
    }

    ListModel {
        id: categoriesModel;
        ListElement {
            name: qsTr("Calm and Relaxed");
            category: "relaxed";
        }
        ListElement {
            name: qsTr("Fast and Excited");
            category: "excited";
        }
        ListElement {
            name: qsTr("Frustrated and Tense");
            category: "tense";
        }
        ListElement {
            name: qsTr("LED Lights");
            category: "lights";
        }
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            Repeater {
        //         property int cellCount: root.width > root.height ? 2 : 1;
        //         cellWidth: poseGrid.width / cellCount; cellHeight: cellWidth * 0.6;
                model: connectionManager ? categoriesModel : null;
        //         implicitWidth: 
                delegate: categoryDelegate;
            }
        }
    }
}
