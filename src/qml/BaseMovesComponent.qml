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
import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0

Item {
    id: root;

    property var categoriesModel: ListModel { }
    property alias infoText: infoCard.text;
    property alias infoFooter: infoCard.footer;
    // If you don't care about whether a command is available on a device right now,
    // set this property to true (it will also not highlight currently running commands)
    property bool ignoreAvailability: false;

    /**
     * When a command has been selected, this signal will be fired.
     * If there are more than one destination selected, they will be listed
     * in the string list destinations
     * @param command The command to activate
     * @param destinations The list of destination device IDs, or an empty list to send to everywhere
     */
    signal commandActivated(string command, variant destinations);

    height: contents.height;
    property int itemsAcross: pageStack.currentItem.width > pageStack.currentItem.height ? 4 : 3;

    property int commandButtonSize: Kirigami.Units.gridUnit * 5

    Component {
        id: categoryDelegate;
        Item {
            id: categoryRoot;
            Layout.fillWidth: true;
            Layout.minimumHeight: categoryHeading.height + commandGrid.height + Kirigami.Units.largeSpacing * 2
            Layout.maximumHeight: categoryHeading.height + commandGrid.height + Kirigami.Units.largeSpacing * 2
            visible: opacity > 0;
            opacity: commandRepeater.count > 0 ? 1 : 0;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            Kirigami.ShadowedRectangle {
                color: modelData["color"]
                radius: Kirigami.Units.largeSpacing
                anchors.fill: parent
            }
            ColumnLayout {
                height: categoryHeading.height + commandGrid.height
                spacing: 0
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
                Kirigami.Heading {
                    id: categoryHeading
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.largeSpacing
                    text: modelData["name"]
                    level: 2
                    wrapMode: Text.Wrap;
                }
                GridLayout {
                    id: commandGrid;
                    columns: commandGrid.width / root.commandButtonSize
                    property int rowCount: Math.ceil(filterProxy.count / columns)
                    Layout.fillWidth: true
                    Layout.minimumHeight: root.commandButtonSize * rowCount
                    Layout.maximumHeight: root.commandButtonSize * rowCount
                    rowSpacing: 0
                    columnSpacing: 0
                    FilterProxyModel {
                        id: filterProxy;
                        sourceModel: CommandModel;
                        filterRole: 260; // the Category role ID
                        filterString: modelData["category"];
                    }
                    Component {
                        id: commandDelegate
                        Item {
                            Layout.fillWidth: true
                            Layout.minimumHeight: root.commandButtonSize
                            Layout.maximumHeight: root.commandButtonSize
                            opacity: model.isAvailable || model.isRunning ? 1 : 0.5;
                            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; } }
                            Rectangle {
                                anchors {
                                    horizontalCenter: parent.horizontalCenter
                                    margins: Kirigami.Units.smallSpacing;
                                }
                                height: root.commandButtonSize - Kirigami.Units.smallSpacing * 2
                                width: height
                                border {
                                    width: model.isRunning ? (root.ignoreAvailability ? 0 : 1) : 0;
                                    color: "silver";
                                }
                                radius: width / 2
                                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                                Label {
                                    anchors {
                                        fill: parent;
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
                                    onClicked: { sendCommandToSelector.selectDestination(command); }
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
                    }
                    Repeater {
                        id: commandRepeater;
                        model: filterProxy;
                        delegate: commandDelegate;
                    }
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
    Kirigami.OverlaySheet {
        id: sendCommandToSelector;
        function selectDestination(command) {
            sendCommandToSelector.command = command;
            if (selectorDeviceModel.count === 1) {
                // If there's only one device, simply assume that's what to send the command to
                root.commandActivated(sendCommandToSelector.command, []);
            } else {
                sendCommandToSelector.open();
            }
        }
        showCloseButton: true;
        property string command;
        header: Kirigami.Heading {
            text: i18nc("Header for the overlay for selecting the destination for a command", "Send where?");
        }
        Item {
            implicitWidth: Kirigami.Units.gridUnit * 30
            height: childrenRect.height
            ColumnLayout {
                spacing: 0;
                anchors { left: parent.left; right: parent.right; }
                InfoCard {
                    text: i18nc("Text for the overlay for selecting the destination for a command", "Pick from the list below where you want to send the command.");
                    Layout.fillWidth: true;
                }
                Repeater {
                    model: FilterProxyModel {
                        id: selectorDeviceModel;
                        sourceModel: DeviceModel;
                        filterRole: 262; // the isConnected role
                        filterBoolean: true;
                        property bool hasCheckedIDs: true;
                        function updateHasCheckedIDs() {
                            var hasChecked = false;
                            for (var i = 0; i < count; ++i) {
                                if(data(index(i, 0), 264) == true) { // if checked
                                    hasChecked = true;
                                    break;
                                }
                            }
                            hasCheckedIDs = hasChecked;
                        }
                        function checkedIDs() {
                            var theIDs = new Array();
                            for (var i = 0; i < count; ++i) {
                                if(data(index(i, 0), 264) == true) { // if checked
                                    theIDs.push(data(index(i, 0), 258)); // add the device ID
                                }
                            }
                            return theIDs;
                        }
                    }
                    Kirigami.BasicListItem {
                        id: deviceListItem;
                        Layout.fillWidth: true;
                        text: model.name ? model.name : "";
                        icon: model.checked ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        property bool itemIsChecked: model.checked !== undefined ? model.checked : false;
                        onItemIsCheckedChanged: { selectorDeviceModel.updateHasCheckedIDs(); }
                        onClicked: { BTConnectionManager.setDeviceChecked(model.deviceID, !model.checked); }
                    }
                }
                Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
                RowLayout {
                    Layout.fillWidth: true;
                    Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
                    Button {
                        text: i18nc("Action for sending a command to the selected devices in a list", "Send To Selected");
                        enabled: selectorDeviceModel.hasCheckedIDs;
                        onClicked: {
                            root.commandActivated(sendCommandToSelector.command, selectorDeviceModel.checkedIDs());
                            sendCommandToSelector.close();
                        }
                    }
                    Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
                    Button {
                        text: i18nc("Action for sending a command to all devices in a list", "Send To All");
                        onClicked: {
                            root.commandActivated(sendCommandToSelector.command, []);
                            sendCommandToSelector.close();
                        }
                    }
                    Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
                }
            }
        }
    }
}
