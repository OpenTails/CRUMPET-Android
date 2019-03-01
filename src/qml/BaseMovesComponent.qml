/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.5 as Kirigami
import org.thetailcompany.digitail 1.0

Item {
    id: control;

    property QtObject categoriesModel: ListModel { }
    property alias blockOnMovingTail: activeMovePopup.blockOnMovingTail;

    signal commandActivated(string command, string name);

    height: contents.height;

    Component {
        id: categoryDelegate;
        Kirigami.AbstractCard {
            Layout.fillWidth: true;
            opacity: commandGrid.count > 0 ? 1 : 0;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            background: Rectangle {
                color: model.color
                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 0
                    verticalOffset: 1
                    radius: 12
                    samples: 32
                    color: Qt.rgba(0, 0, 0, 0.5)
                }
            }
            header: Kirigami.Heading {
                text: model.name
                level: 2
            }
            contentItem: Item {
                implicitHeight: commandGrid.cellHeight * Math.ceil(commandGrid.count / 3);
                FilterProxyModel {
                    id: filterProxy;
                    sourceModel: CommandModel;
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
                            Kirigami.Theme.colorSet: Kirigami.Theme.Button
                        }
                        Label {
                            anchors {
                                fill: parent;
                                margins: Kirigami.Units.smallSpacing * 2;
                            }
                            wrapMode: Text.Wrap;
                            horizontalAlignment: Text.AlignHCenter;
                            verticalAlignment: Text.AlignVCenter;
                            text: model.name ? model.name : "";
                            Kirigami.Theme.colorSet: Kirigami.Theme.Button
                        }
                        MouseArea {
                            anchors.fill: parent;
                            // this is An Hack (for some reason the model replication is lossy on first attempt, but we shall live)
                            property string command: model.command ? model.command : "";
                            onClicked: { control.commandActivated(command, model.name); }
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

    Item {
        id: contents;
        width: parent.width;
        height: mainLayout.height;
        ColumnLayout {
            width: parent.width;
            id: mainLayout;
            Item {
                implicitHeight: tailConnectedInfo.opacity > 0 ? tailConnectedInfo.implicitHeight : 0;
                Layout.fillWidth: true;
                NotConnectedCard {
                    id: tailConnectedInfo;
                }
            }
            Kirigami.CardsLayout {
                Layout.fillWidth: true;
                Layout.fillHeight: true;
                Repeater {
                    model: BTConnectionManager.isConnected ? categoriesModel : null;
                    delegate: categoryDelegate;
                }
            }
        }
    }

    ActiveMovePopup {
        id: activeMovePopup

        x: (root.width - width) / 2
        y: (root.height - height) / 2

        Connections {
            target: control

            onCommandActivated: {
                activeMovePopup.commandName = name;
            }
        }
    }
}
