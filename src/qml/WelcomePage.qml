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
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.4 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    id: root;
    objectName: "welcomePage";
    title: qsTr("Crumpet");
    actions {
        main: Kirigami.Action {
            text: BTConnectionManager.isConnected ? "Disconnect" : "Connect";
            icon.name: BTConnectionManager.isConnected ? "network-disconnect" : "network-connect";
            onTriggered: {
                if(BTConnectionManager.isConnected) {
                    BTConnectionManager.disconnectDevice("");
                }
                else {
                    if(BTConnectionManager.deviceCount === 1) {
                        BTConnectionManager.stopDiscovery();
                    }
                    else {
                        connectToTail.open();
                    }
                }
            }
        }
        right: (BTConnectionManager.isConnected && DeviceModel.rowCount() > 1) ? connectMoreAction : null
    }
    property QtObject connectMoreAction: Kirigami.Action {
        text: qsTr("Connect More...");
        icon.name: "list-add";
        onTriggered: connectToTail.open();
    }

    ScrollView {
        ColumnLayout {
            width: root.width - Kirigami.Units.largeSpacing * 4;
            TailBattery {
                width: parent.width;
            }
            NotConnectedCard { }
            Item { height: Kirigami.Units.smallSpacing; width: parent.width; }
            Kirigami.AbstractCard {
                contentItem: ColumnLayout {
                    Kirigami.BasicListItem {
                        text: qsTr("Moves");
                        icon: ":/images/tail_moves.svg";
                        separatorVisible: false;
                        onClicked: {
                            switchToPage(tailMoves);
                        }
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight;
                            Layout.margins: Kirigami.Units.smallSpacing;
                            width: Kirigami.Units.iconSizes.small;
                            height: width;
                            source: "go-next";
                        }
                    }
                    Kirigami.BasicListItem {
                        text: qsTr("Glow Tips");
                        icon: ":/images/tail_lights.svg";
                        separatorVisible: false;
                        onClicked: {
                            switchToPage(tailLights);
                        }
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight;
                            Layout.margins: Kirigami.Units.smallSpacing;
                            width: Kirigami.Units.iconSizes.small;
                            height: width;
                            source: "go-next";
                        }
                    }
                    Item { height: Kirigami.Units.smallSpacing; width: parent.width; }
                    Kirigami.BasicListItem {
                        text: qsTr("Alarm");
                        icon: "accept_time_event";
                        separatorVisible: false;
                        onClicked: {
                            switchToPage(alarmList);
                        }
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight;
                            Layout.margins: Kirigami.Units.smallSpacing;
                            width: Kirigami.Units.iconSizes.small;
                            height: width;
                            source: "go-next";
                        }
                    }
                    Kirigami.BasicListItem {
                        text: qsTr("Move List");
                        icon: "view-media-playlist";
                        separatorVisible: false;
                        onClicked: {
                            switchToPage(moveLists);
                        }
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight;
                            Layout.margins: Kirigami.Units.smallSpacing;
                            width: Kirigami.Units.iconSizes.small;
                            height: width;
                            source: "go-next";
                        }
                    }
                    Item { height: Kirigami.Units.smallSpacing; width: parent.width; }
                    Kirigami.BasicListItem {
                        text: qsTr("Poses");
                        icon: ":/images/tail.svg";
                        separatorVisible: false;
                        onClicked: {
                            showPassiveNotification(qsTr("Sorry, nothing yet..."), 1500);
                        }
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight;
                            Layout.margins: Kirigami.Units.smallSpacing;
                            width: Kirigami.Units.iconSizes.small;
                            height: width;
                            source: "go-next";
                        }
                    }
                }
            }
            Item { height: Kirigami.Units.smallSpacing; width: parent.width; }
            Kirigami.AbstractCard {
                opacity: BTConnectionManager.isConnected ? 1 : 0;
                Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                width: parent.width;
                header: Kirigami.Heading {
                    text: qsTr("Casual Mode");
                    CheckBox {
                        anchors.right: parent.right;
                        height: parent.height;
                        width: height;
                        checked: AppSettings.idleMode;
                        onClicked: { AppSettings.idleMode = !AppSettings.idleMode; }
                    }
                }
                Component {
                    id: casualModeSettingsListItem
                    Kirigami.BasicListItem {
                            text: qsTr("Casual Mode Settings");
                            width: parent.width;
                            Layout.fillWidth: true;
                            separatorVisible: false;
                            icon: "settings-configure";
                            onClicked: switchToPage(idleModePage);
                        }
                }
                Component {
                    id: idlePauseRangePicker;

                    ColumnLayout {
                        Layout.fillWidth: true;
                        IdlePauseRangePicker {
                        }
                        Loader {
                            Layout.fillWidth: true;
                            sourceComponent: casualModeSettingsListItem
                        }
                    }
                }
                Component {
                    id: emptyNothing;
                    Loader {
                        sourceComponent: casualModeSettingsListItem
                    }
                }
                contentItem: Loader {
                    sourceComponent: AppSettings.idleMode === true ? idlePauseRangePicker : emptyNothing;
                }
            }
    //         Button {
    //             text: qsTr("Tailkiller! Slow Wag 1 + 3sec pause loop");
    //             width: parent.width;
    //             onClicked: {
    //                 for(var i = 0; i < 1000; ++i) {
    //                     CommandQueue.pushCommand(CommandModel.getCommand(1));
    //                     CommandQueue.pushPause(3000);
    //                 }
    //             }
    //         }
        }
    }
}
