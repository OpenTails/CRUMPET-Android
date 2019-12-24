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
    id: root;

    property QtObject categoriesModel: ListModel { }
    property alias infoText: infoCard.text;
    // If you don't care about whether a command is available on a device right now,
    // set this property to true (it will also not highlight currently running commands)
    property bool ignoreAvailability: false;

    signal commandActivated(string command, string commandName);

    height: contents.height;
    property int itemsAcross: pageStack.currentItem.width > pageStack.currentItem.height ? 4 : 3;

    Component {
        id: categoryDelegate;
        Kirigami.AbstractCard {
            id: categoryRoot;
            Layout.fillWidth: true;
            opacity: commandRepeater.count > 0 ? 1 : 0;
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
            contentItem: Flow {
                id: commandGrid;
                width: categoryRoot.width - (Kirigami.Units.largeSpacing * 4)
                FilterProxyModel {
                    id: filterProxy;
                    sourceModel: CommandModel;
                    filterRole: 260; // the Category role ID
                    filterString: model.category;
                }
                Component {
                    id: commandDelegate
                    Item {
                        width: commandGrid.width / root.itemsAcross;
                        height: width;
                        opacity: model.isAvailable ? 1 : 0.5;
                        Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; } }
                        Rectangle {
                            anchors {
                                fill: parent;
                                margins: Kirigami.Units.smallSpacing;
                            }
                            border {
                                width: model.isRunning ? (root.ignoreAvailability ? 0 : 1) : 0;
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
                            onClicked: { root.commandActivated(command, model.name); }
                            enabled: root.ignoreAvailability || (typeof model.isAvailable !== "undefined" ? model.isAvailable : false);
                        }
                        BusyIndicator {
                            anchors {
                                fill: parent;
                                margins: Kirigami.Units.smallSpacing;
                            }
                            opacity: model.isRunning ? (root.ignoreAvailability ? 0 : 1) : 0;
                            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; } }
                            running: opacity > 0
                        }
                    }
                }
                Repeater {
                    id: commandRepeater;
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
            id: mainLayout;
            width: parent.width;
            spacing: Kirigami.Units.largeSpacing;
            Item {
                implicitHeight: tailConnectedInfo.opacity > 0 ? tailConnectedInfo.implicitHeight : infoCard.height;
                Layout.fillWidth: true;
                NotConnectedCard {
                    id: tailConnectedInfo;
                    anchors {
                        top: parent.top;
                        left: parent.left;
                    }
                }
                InfoCard {
                    id: infoCard;
                    anchors {
                        top: parent.top;
                        left: parent.left;
                    }
                    opacity: tailConnectedInfo.opacity === 0 ? 1 : 0;
                    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                }
            }
            Repeater {
                model: BTConnectionManager.isConnected ? categoriesModel : null;
                delegate: categoryDelegate;
            }
        }
    }
}
