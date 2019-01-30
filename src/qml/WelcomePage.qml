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

Kirigami.Page {
    id: root;
    objectName: "welcomePage";
    title: qsTr("DIGITAiL");
    actions {
        main: Kirigami.Action {
            text: BTConnectionManager.isConnected ? "Disconnect" : "Connect";
            icon.name: BTConnectionManager.isConnected ? ":/org/kde/kirigami/icons/network-disconnect.svg" : ":/org/kde/kirigami/icons/network-connect.svg";
            onTriggered: {
                if(BTConnectionManager.isConnected) {
                    BTConnectionManager.disconnectDevice();
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
        contextualActions: [
            Kirigami.Action {
                text: qsTr("Advanced Options");
                icon.name: AppSettings.advancedMode ? ":/org/kde/kirigami/icons/checkbox-checked.svg" : ":/org/kde/kirigami/icons/checkbox-unchecked.svg";
                onTriggered: { AppSettings.advancedMode = !AppSettings.advancedMode; }
            }
        ]
    }

    Column {
        width: root.width - Kirigami.Units.largeSpacing * 4;
        spacing: Kirigami.Units.largeSpacing;
        NotConnectedCard { }
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
                    showPassiveNotification(qsTr("Link a command to an alarm (but not yet, sorry)"), 1500);
                }
            }
            Button {
                Layout.column: 0;
                Layout.row: 1;
                Layout.fillWidth: true; Layout.fillHeight: true;
                text: qsTr("Tail Poses");
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter;
                onClicked: {
                    showPassiveNotification(qsTr("Sorry, nothing yet..."), 1500);
                }
            }
            Button {
                Layout.column: 1;
                Layout.row: 0;
                Layout.fillWidth: true; Layout.fillHeight: true;
                text: qsTr("Move List");
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter;
                onClicked: {
                    showPassiveNotification(qsTr("Queue up multiple moves and commands (just not yet...)"), 1500);
                }
            }
            Button {
                Layout.column: 1;
                Layout.row: 1;
                Layout.fillWidth: true; Layout.fillHeight: true;
                text: qsTr("Tail Lights");
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter;
                onClicked: {
                    switchToPage(tailLights);
                }
            }
            Button {
                text: qsTr("Tail Moves");
                Layout.column: commandLayout.columns === 2 ? 0 : 2;
                Layout.row: commandLayout.columns === 2 ? 2 : 0;
                Layout.columnSpan: 2;
                Layout.rowSpan: 2;
                Layout.fillWidth: true; Layout.fillHeight: true;
                onClicked: {
                    switchToPage(tailMoves);
                }
            }
        }
        Kirigami.AbstractCard {
            opacity: BTConnectionManager.isConnected ? 1 : 0;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            width: parent.width;
            header: Kirigami.Heading {
                text: qsTr("Idle Mode");
                CheckBox {
                    anchors.right: parent.right;
                    height: parent.height;
                    width: height;
                    checked: AppSettings.idleMode;
                    onClicked: { AppSettings.idleMode = !AppSettings.idleMode; }
                    RoundButton {
                        opacity: parent.checked ? 1 : 0;
                        Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                        anchors {
                            right: parent.left;
                            rightMargin: Kirigami.Units.smallSpacing;
                            verticalCenter: parent.verticalCenter;
                        }
                        height: parent.height;
                        width: height;
                        onClicked: switchToPage(idleModePage);
                        Kirigami.Icon {
                            source: "settings-configure";
                            anchors.fill: parent;
                        }
                    }
                }
            }
            Component {
                id: idlePauseRangePicker;
                Column {
                    Label {
                        text: qsTr("Range of pause between random moves in seconds");
                        anchors { left: parent.left; right: parent.right; }
                    }
                    RangeSlider {
                        id: pauseRangeSlider;
                        anchors { left: parent.left; right: parent.right; leftMargin: Kirigami.Units.largeSpacing; }
                        from: 0;
                        to: 120;
                        stepSize: 1.0;
                        first.onValueChanged: { AppSettings.idleMinPause = first.value; }
                        second.onValueChanged: { AppSettings.idleMaxPause = second.value; }
                        Component.onCompleted: {
                            pauseRangeSlider.setValues(AppSettings.idleMinPause, AppSettings.idleMaxPause);
                        }
                    }
                    Item {
                        anchors { left: parent.left; right: parent.right; leftMargin: Kirigami.Units.largeSpacing; }
                        height: childrenRect.height;
                        Label {
                            text: Math.floor(pauseRangeSlider.first.value);
                            anchors {
                                left: parent.left;
                                right: parent.horizontalCentre;
                            }
                        }
                        Label {
                            text: Math.floor(pauseRangeSlider.second.value);
                            verticalAlignment: Text.AlignRight;
                            anchors {
                                left: parent.horizontalCentre;
                                right: parent.right;
                            }
                        }
                    }
                }
            }
            Component {
                id: emptyNothing;
                Item {}
            }
            contentItem: Loader {
                sourceComponent: AppSettings.idleMode === true ? idlePauseRangePicker : emptyNothing;
            }
        }
        Button {
            text: qsTr("Tailkiller! Slow Wag 1 + 3sec pause loop");
            width: parent.width;
            onClicked: {
                for(var i = 0; i < 1000; ++i) {
                    CommandQueue.pushCommand(CommandModel.getCommand(1));
                    CommandQueue.pushPause(3000);
                }
            }
        }
    }
}
