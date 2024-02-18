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
 *   along with this program; if not, see <httpsqrc://www.gnu.org/licenses/>
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.ScrollablePage {
    id: root;
    objectName: "welcomePage";
    title: i18nc("Header for the welcome page", "Crumpet");
    actions: [
        Kirigami.Action {
            text: Digitail.BTConnectionManager.isConnected ? i18nc("Label for the button for disconnecting gear, on the welcome page", "Disconnect") : i18nc("Label for the button for connecting gear, on the welcome page","Connect");
            icon.name: Digitail.BTConnectionManager.isConnected ? "network-disconnect" : "network-connect";
            onTriggered: {
                if(Digitail.BTConnectionManager.isConnected) {
                    if(Digitail.BTConnectionManager.deviceCount === 1) {
                        disconnectionOptions.disconnectGear(connectedDevicesModel.data(connectedDevicesModel.index(0, 0), Digitail.DeviceModelTypes.DeviceID));
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
        },
        Kirigami.Action {
            text: i18nc("Label for the button for looking for additional gear, on the welcome page", "Look for gear");
            icon.name: "view-refresh";
            onTriggered: {
                Digitail.BTConnectionManager.startDiscovery();
            }
            visible: Digitail.BTConnectionManager.discoveryRunning === false
        },
        Kirigami.Action {
            text: i18nc("Label for the button for connecting additional gear, on the welcome page", "Connect More...");
            icon.name: "list-add";
            onTriggered: {
                connectToTail.open();
            }
            visible: Digitail.BTConnectionManager.isConnected && allDevicesModel.count > 1
        }
    ]
    property QtObject allDevicesModel: Digitail.FilterProxyModel {
        sourceModel: Digitail.DeviceModel;
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
                BasicListItem {
                    Layout.fillWidth: true
                    text: i18nc("Label for the button for opening the Moves page, on the welcome page", "Moves");
                    visible: opacity > 0;
                    opacity: connectedDevicesModel.count > 0 ? 1 : 0;
                    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                    icon.source: "qrc:/images/moves.svg";
                    onClicked: {
                        switchToPage(tailMoves);
                    }
                    Kirigami.Icon {
                        anchors {
                            top: parent.top
                            right: parent.right
                            bottom: parent.bottom
                            margins: Kirigami.Units.largeSpacing;
                        }
                        width: height
                        source: "go-next";
                    }
                }
                BasicListItem {
                    Layout.fillWidth: true
                    text: i18nc("Label for the button for opening the Ear Poses page, on the welcome page", "Ear Poses");
                    visible: opacity > 0;
                    opacity: hasListeningDevicesRepeater.count > 0 ? 1 : 0;
                    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                    icon.source: "qrc:/images/earposes.svg";
                    onClicked: {
                        switchToPage(earPoses);
                    }
                    Kirigami.Icon {
                        anchors {
                            top: parent.top
                            right: parent.right
                            bottom: parent.bottom
                            margins: Kirigami.Units.largeSpacing;
                        }
                        width: Kirigami.Units.iconSizes.small;
                        height: width;
                        source: "go-next";
                    }
                }
                BasicListItem {
                    Layout.fillWidth: true
                    text: i18nc("Label for the button for opening the Glow Tips page, on the welcome page", "Glow Tips");
                    visible: opacity > 0;
                    opacity: connectedWithLightsModel.count > 0;
                    Behavior on opacity { PropertyAnimation { duration: Kirigami.Units.shortDuration; } }
                    icon.source: "qrc:/images/glowtip.svg";
                    onClicked: {
                        switchToPage(tailLights);
                    }
                    Kirigami.Icon {
                        anchors {
                            top: parent.top
                            right: parent.right
                            bottom: parent.bottom
                            margins: Kirigami.Units.largeSpacing;
                        }
                        width: height
                        source: "go-next";
                    }
                    Digitail.FilterProxyModel {
                        id: connectedWithLightsModel;
                        sourceModel: connectedDevicesModel;
                        filterRole: Digitail.DeviceModelTypes.HasLights;
                        filterBoolean: true;
                    }
                }
                Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; visible: connectedDevicesModel.count > 0; }
                BasicListItem {
                    Layout.fillWidth: true
                    text: i18nc("Label for the button for opening the Alarms page, on the welcome page", "Alarm");
                    icon.source: "qrc:/images/alarm.svg";
                    onClicked: {
                        switchToPage(alarmList);
                    }
                    Kirigami.Icon {
                        anchors {
                            top: parent.top
                            right: parent.right
                            bottom: parent.bottom
                            margins: Kirigami.Units.largeSpacing;
                        }
                        width: height
                        source: "go-next";
                    }
                }
                BasicListItem {
                    Layout.fillWidth: true
                    text: i18nc("Label for the button for opening the Move List page, on the welcome page", "Move List");
                    icon.source: "qrc:/images/movelist.svg";
                    onClicked: {
                        switchToPage(moveLists);
                    }
                    Kirigami.Icon {
                        anchors {
                            top: parent.top
                            right: parent.right
                            bottom: parent.bottom
                            margins: Kirigami.Units.largeSpacing;
                        }
                        width: height
                        source: "go-next";
                    }
                }
//                     Item { height: Kirigami.Units.smallSpacing; Layout.fillWidth: true; }
//                     BasicListItem {
//                         Layout.fillWidth: true
//                         text: i18nc("Label for the button for opening the Poses page, on the welcome page", "Poses");
//                         icon.source: "qrc:/images/tail.svg";
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
                        width: Kirigami.Units.iconSizes.medium;
                        height: width;
                        source: "qrc:/images/casualmode.svg"
                        TapHandler {
                            onTapped: {
                                switchToPage(idleModePage);
                            }
                        }
                    }
                    Kirigami.Heading {
                        text: i18nc("Label for the button for opening the Casual Mode page, on the welcome page", "Casual Mode");
                        Layout.fillWidth: true;
                        wrapMode: Text.Wrap;
                        TapHandler {
                            onTapped: {
                                switchToPage(idleModePage);
                            }
                        }
                    }
                    ToolButton {
                        onClicked: {
                            switchToPage(idleModePage);
                        }
                        icon.name: "go-next";
                    }
            }
            Component {
                id: casualModeSettingsListItem
                BasicListItem {
                    Layout.fillWidth: true
                    text: i18nc("Label for the button for enabling the Casual Mode, on the welcome page", "Enable Casual Mode");
                    icon.source: (Digitail.AppSettings !== null && Digitail.AppSettings.idleMode) ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
                    onClicked: { Digitail.AppSettings.idleMode = !Digitail.AppSettings.idleMode; }
                }
            }
            Component {
                id: idlePauseRangePicker;
                ColumnLayout {
                    Layout.fillWidth: true;
                    Loader {
                        Layout.fillWidth: true;
                        sourceComponent: casualModeSettingsListItem
                    }
                    IdlePauseRangePicker {
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
                    width: Kirigami.Units.iconSizes.medium;
                    height: width;
                    source: "qrc:/images/listeningmode.svg"
                    TapHandler {
                        enabled: Digitail.AppSettings.developerMode
                        onTapped: {
                            switchToPage(listenSettings);
                        }
                    }
                }
                Kirigami.Heading {
                    text: i18nc("Heading for the panel for turning on Listening Mode", "Listening Mode");
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    Digitail.FilterProxyModel {
                        id: connectedDevicesModel
                        sourceModel: Digitail.DeviceModel;
                        filterRole: Digitail.DeviceModelTypes.IsConnected;
                        filterBoolean: true;
                    }
                    TapHandler {
                        enabled: Digitail.AppSettings.developerMode
                        onTapped: {
                            switchToPage(listenSettings);
                        }
                    }
                }
                ToolButton {
                    visible: Digitail.AppSettings.developerMode
                    onClicked: {
                        switchToPage(listenSettings);
                    }
                    icon.name: "go-next";
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
                        filterRole: Digitail.DeviceModelTypes.HasListening;
                        filterBoolean: true;
                    }
                    BasicListItem {
                        width: listeningColumn.width;
                        icon.source: model.listeningState > 0 ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        text: model.name;
                        onClicked: {
                            var newState = 0;
                            if (model.listeningState == 0) {
                                newState = 1;
                                for (let commandIndex = 0; commandIndex < model.gestureEventCommands.length; ++commandIndex) {
                                    if (model.supportedSoundEvents.includes(model.gestureEventValues[commandIndex]) && model.gestureEventCommands[commandIndex].length > 0) {
                                        // If any of the device's supported sound events have a command set, turn on full listening mode, otherwise just turn on standard
                                        newState = 2;
                                        break;
                                    }
                                }
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
                    source: "qrc:/images/tiltmode.svg"
                    TapHandler {
                        enabled: Digitail.AppSettings.developerMode
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
                        enabled: Digitail.AppSettings.developerMode
                        onTapped: {
                            switchToPage(tiltSettings);
                        }
                    }
                }
                ToolButton {
                    visible: Digitail.AppSettings.developerMode
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
                        filterRole: Digitail.DeviceModelTypes.HasTilt;
                        filterBoolean: true;
                    }
                    BasicListItem {
                        width: tiltingColumn.width;
                        icon.source: model.tiltEnabled === true ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        text: model.name;
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
                    source: "qrc:/images/movelist.svg"
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
                        filterRole: Digitail.GestureDetectorModelTypes.SensorPinnedRole;
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
                        filterRole: Digitail.GestureDetectorModel.FirstInSensorRole;
                        filterBoolean: true;
                    }
                    BasicListItem {
                        visible: model.firstInSensor;
                        width: gesturesColumn.width;
                        icon.source: model.sensorEnabled > 0 ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        text: model.sensorName;
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
