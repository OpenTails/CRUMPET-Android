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

import QtQuick 2.14
import QtQuick.Controls 2.11
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    id: root;
    objectName: "welcomePage";
    title: i18nc("Header for the welcome page", "Crumpet");
    actions {
        main: Kirigami.Action {
            text: Digitail.BTConnectionManager.isConnected ? i18nc("Label for the button for disconnecting gear, on the welcome page", "Disconnect") : i18nc("Label for the button for connecting gear, on the welcome page","Connect");
            icon.name: Digitail.BTConnectionManager.isConnected ? "network-disconnect" : "network-connect";
            onTriggered: {
                if(Digitail.BTConnectionManager.isConnected) {
                    if(Digitail.BTConnectionManager.deviceCount === 1) {
                        disconnectionOptions.disconnectGear(connectedDevicesModel.data(connectedDevicesModel.index(0, 0), 258));
                    }
                    else if (Digitail.BTConnectionManager.deviceCount > 1) {
                        connectToTail.open();
                    }
                }
                else {
                    if(Digitail.BTConnectionManager.deviceCount === 1) {
                        Digitail.BTConnectionManager.stopDiscovery();
                    }
                    else if (Digitail.BTConnectionManager.deviceCount > 1) {
                        connectToTail.open();
                    }
                }
            }
        }
        left: Digitail.BTConnectionManager.discoveryRunning ? null : searchForMoreAction
        right: (Digitail.BTConnectionManager.isConnected && allDevicesModel.count > 1) ? connectMoreAction : null
    }
    property QtObject allDevicesModel: Digitail.FilterProxyModel {
        sourceModel: Digitail.DeviceModel;
    }
    property QtObject connectMoreAction: Kirigami.Action {
        text: i18nc("Label for the button for connecting additional gear, on the welcome page", "Connect More...");
        icon.name: "list-add";
        onTriggered: connectToTail.open();
    }
    property QtObject searchForMoreAction: Kirigami.Action {
        text: i18nc("Label for the button for looking for additional gear, on the welcome page", "Look for gear");
        icon.name: "view-refresh";
        onTriggered: Digitail.BTConnectionManager.startDiscovery();
    }

    ColumnLayout {
        width: root.width - Kirigami.Units.largeSpacing * 2
        TailBattery {
            Layout.fillWidth: true;
        }
        NotConnectedCard { }
        Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
        Kirigami.AbstractCard {
            contentItem: ColumnLayout {
                Kirigami.BasicListItem {
                    text: i18nc("Label for the button for opening the Moves page, on the welcome page", "Moves");
                    visible: opacity > 0;
                    opacity: connectedDevicesModel.count > 0 ? 1 : 0;
                    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                    icon: ":/images/moves.svg";
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
                    text: i18nc("Label for the button for opening the Ear Poses page, on the welcome page", "Ear Poses");
                    visible: opacity > 0;
                    opacity: hasListeningDevicesRepeater.count > 0 ? 1 : 0;
                    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                    icon: ":/images/earposes.svg";
                    separatorVisible: false;
                    onClicked: {
                        switchToPage(earPoses);
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
                    text: i18nc("Label for the button for opening the Glow Tips page, on the welcome page", "Glow Tips");
                    visible: opacity > 0;
                    opacity: connectedWithLightsModel.count > 0;
                    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                    icon: ":/images/glowtip.svg";
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
                    Digitail.FilterProxyModel {
                        id: connectedWithLightsModel;
                        sourceModel: connectedDevicesModel;
                        filterRole: 276; // the hasLights role
                        filterBoolean: true;
                    }
                }
                Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; visible: connectedDevicesModel.count > 0; }
                Kirigami.BasicListItem {
                    text: i18nc("Label for the button for opening the Alarms page, on the welcome page", "Alarm");
                    icon: ":/images/alarm.svg";
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
                    text: i18nc("Label for the button for opening the Move List page, on the welcome page", "Move List");
                    icon: ":/images/movelist.svg";
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
//                     Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
//                     Kirigami.BasicListItem {
//                         text: i18nc("Label for the button for opening the Poses page, on the welcome page", "Poses");
//                         icon: ":/images/tail.svg";
//                         separatorVisible: false;
//                         onClicked: {
//                             showPassiveNotification(i18nc("Warning for the missing functionality of the Poses page", "Sorry, nothing yet..."), 1500);
//                         }
//                         Kirigami.Icon {
//                             Layout.alignment: Qt.AlignVCenter | Qt.AlignRight;
//                             Layout.margins: Kirigami.Units.smallSpacing;
//                             width: Kirigami.Units.iconSizes.small;
//                             height: width;
//                             source: "go-next";
//                         }
//                     }
            }
        }
        Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
        Kirigami.AbstractCard {
            visible: opacity > 0;
            opacity: Digitail.BTConnectionManager.isConnected ? 1 : 0;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            Layout.fillWidth: true;
            header: RowLayout {
                    Kirigami.Icon {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft;
                        Layout.margins: Kirigami.Units.smallSpacing;
                        width: Kirigami.Units.iconSizes.small;
                        height: width;
                        source: ":/images/casualmode.svg"
                    }
                    Kirigami.Heading {
                        text: i18nc("Label for the button for opening the Casual Mode page, on the welcome page", "Casual Mode");
                        Layout.fillWidth: true;
                        wrapMode: Text.Wrap;
                    }
                    CheckBox {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight;
                        Layout.margins: Kirigami.Units.smallSpacing;
                        height: Kirigami.Units.iconSizes.small;
                        width: height;
                        checked: Digitail.AppSettings !== null ? Digitail.AppSettings.idleMode : false;
                        onClicked: { Digitail.AppSettings.idleMode = !Digitail.AppSettings.idleMode; }
                    }
            }
            Component {
                id: casualModeSettingsListItem
                Kirigami.BasicListItem {
                        text: i18nc("Label for the button for configuring the Casual Mode, on the welcome page", "Casual Mode Settings");
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
                sourceComponent: (Digitail.AppSettings !== null && Digitail.AppSettings.idleMode === true) ? idlePauseRangePicker : emptyNothing;
            }
        }
        Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
        Kirigami.AbstractCard {
            visible: opacity > 0;
            opacity: hasListeningDevicesRepeater.count > 0 ? 1 : 0;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            Layout.fillWidth: true;
            header: RowLayout {
                Kirigami.Icon {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft;
                    Layout.margins: Kirigami.Units.smallSpacing;
                    width: Kirigami.Units.iconSizes.small;
                    height: width;
                    source: ":/images/listeningmode.svg"
                }
                Kirigami.Heading {
                    text: i18nc("Heading for the panel for turning on Listening Mode", "Listening Mode");
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    Digitail.FilterProxyModel {
                        id: connectedDevicesModel
                        sourceModel: Digitail.DeviceModel;
                        filterRole: 262; // the isConnected role
                        filterBoolean: true;
                    }
                }
            }
            contentItem: Column {
                id: listeningColumn;
                Layout.fillWidth: true;
                height: childrenRect.height;
                spacing: 0;
                Label {
                    width: parent.width;
                    wrapMode: Text.Wrap;
                    text: i18nc("Label for the checkbox for turning on Listening Mode", "Turn this on to make your gear react to sounds around it for five minutes at a time.");
                }
                Repeater {
                    id: hasListeningDevicesRepeater;
                    model: Digitail.FilterProxyModel {
                        sourceModel: connectedDevicesModel;
                        filterRole: 265; // the hasListening role
                        filterBoolean: true;
                    }
                    Kirigami.BasicListItem {
                        width: listeningColumn.width;
                        separatorVisible: false;
                        icon: model.listeningState > 0 ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        label: model.name;
                        onClicked: {
                            var newState = 0;
                            if (model.listeningState == 0) {
                                newState = 1;
                            }
                            Digitail.BTConnectionManager.setDeviceListeningState(model.deviceID, newState);
                        }
                    }
                }
            }
        }
        Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
        Kirigami.AbstractCard {
            visible: opacity > 0;
            opacity: hasTiltDevicesRepeater.count > 0 ? 1 : 0;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            Layout.fillWidth: true;
            header: RowLayout {
                Kirigami.Icon {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft;
                    Layout.margins: Kirigami.Units.smallSpacing;
                    width: Kirigami.Units.iconSizes.small;
                    height: width;
                    source: ":/images/tiltmode.svg"
                    TapHandler {
                        onTapped: {
                            switchToPage(tiltSettings);
                        }
                    }
                }
                Kirigami.Heading {
                    text: i18nc("Heading for the panel for turning on Tilt Mode", "Tilt Mode");
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    TapHandler {
                        onTapped: {
                            switchToPage(tiltSettings);
                        }
                    }
                }
                ToolButton {
                    onClicked: {
                        switchToPage(tiltSettings);
                    }
                    icon.name: "go-next";
                }
            }
            contentItem: Column {
                id: tiltingColumn;
                Layout.fillWidth: true;
                height: childrenRect.height;
                spacing: 0;
                Label {
                    width: parent.width;
                    wrapMode: Text.Wrap;
                    text: i18nc("Label for the checkbox for turning on Tilt Mode", "Turn this on to make your gear react to tilting movements.");
                }
                Repeater {
                    id: hasTiltDevicesRepeater;
                    model: Digitail.FilterProxyModel {
                        sourceModel: connectedDevicesModel;
                        filterRole: 282; // the hasTilt role
                        filterBoolean: true;
                    }
                    Kirigami.BasicListItem {
                        width: tiltingColumn.width;
                        separatorVisible: false;
                        icon: model.tiltEnabled === true ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        label: model.name;
                        onClicked: {
                            Digitail.BTConnectionManager.setDeviceTiltState(model.deviceID, !model.tiltEnabled);
                        }
                    }
                }
            }
        }
        Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
        Kirigami.AbstractCard {
            visible: opacity > 0;
            opacity: Digitail.BTConnectionManager.isConnected ? 1 : 0;
            Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
            Layout.fillWidth: true;
            header: RowLayout {
                Kirigami.Icon {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft;
                    Layout.margins: Kirigami.Units.smallSpacing;
                    width: Kirigami.Units.iconSizes.small;
                    height: width;
                    source: ":/images/movelist.svg"
                    TapHandler {
                        onTapped: {
                            switchToPage(gearGestures);
                        }
                    }
                }
                Kirigami.Heading {
                    text: i18nc("Label for the button for opening the Gear Gestures page, on the welcome page", "Gear Gestures");
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    Digitail.FilterProxyModel {
                        id: pinnedSensorsModel;
                        sourceModel: Digitail.GestureDetectorModel;
                        filterRole: 262; // the sensorPinned role
                        filterBoolean: true;
                    }
                    TapHandler {
                        onTapped: {
                            switchToPage(gearGestures);
                        }
                    }
                }
                ToolButton {
                    onClicked: {
                        switchToPage(gearGestures);
                    }
                    icon.name: "go-next";
                }
            }
            contentItem: Column {
                id: gesturesColumn
                Layout.fillWidth: true;
                height: childrenRect.height;
                spacing: 0;
                Repeater {
                    model: Digitail.FilterProxyModel {
                        id: welcomePageSensorsModel
                        sourceModel: pinnedSensorsModel
                        filterRole: 266; // the firstInSensor role
                        filterBoolean: true;
                    }
                    Kirigami.BasicListItem {
                        visible: model.firstInSensor;
                        width: gesturesColumn.width;
                        separatorVisible: false;
                        icon: model.sensorEnabled > 0 ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        label: model.sensorName;
                        onClicked: { Digitail.GestureController.setGestureSensorEnabled(pinnedSensorsModel.sourceIndex(welcomePageSensorsModel.sourceIndex(model.index)), !model.sensorEnabled); }
                    }
                }
            }
        }

//         Button {
//             text: i18nc("Button for executing the killer poke cycle, intended to destroy your gear", "Tailkiller! Slow Wag 1 + 3sec pause loop");
//             Layout.fillWidth: true;
//             onClicked: {
//                 for(var i = 0; i < 1000; ++i) {
//                     CommandQueue.pushCommand(CommandModel.getCommand(1));
//                     CommandQueue.pushPause(3000);
//                 }
//             }
//         }
    }
}
