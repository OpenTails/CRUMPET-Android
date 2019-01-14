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
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.5 as Kirigami
import org.thetailcompany.digitail 1.0

Kirigami.ScrollablePage {
    id: root;
    title: qsTr("Tail Moves (battery level: %1)").arg(connectionManager.batteryLevel);
    property QtObject categoriesModel: ListModel { }
    property QtObject connectionManager: null;

    Component {
        id: categoryDelegate;
        Kirigami.AbstractCard {
            Layout.fillHeight: true
            width: Kirigami.Units.gridUnit * 30;
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
            footer: CheckBox {
                    id: idleCheck;
                    opacity: AppSettings.idleMode === true ? 1 : 0;
                    text: qsTr("Include in idle mode");
                    checked: {
                        if(AppSettings.idleCategories.includes(model.category)) {
                            return true;
                        }
                        return false;
                    }
                    onClicked: {
                        var categories = AppSettings.idleCategories;
                        if(checked === true) {
                            categories.push(model.category);
                        }
                        else {
                            var idx = categories.indexOf(model.category);
                            categories.splice(idx, 1);
                        }
                        AppSettings.idleCategories = categories;
                    }
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
                            text: model.name;
                            Kirigami.Theme.colorSet: Kirigami.Theme.Button
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

    ColumnLayout {
        Item {
            implicitHeight: tailConnectedInfo.opacity > 0 ? tailConnectedInfo.implicitHeight : 0;
            width: root.width - Kirigami.Units.largeSpacing * 4;
            NotConnectedCard {
                id: tailConnectedInfo;
                connectionManager: root.connectionManager;
            }
        }
        Kirigami.CardsLayout {
            Repeater {
                model: connectionManager ? categoriesModel : null;
                delegate: categoryDelegate;
            }
        }
    }
}
