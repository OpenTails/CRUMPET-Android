/*
 *   Copyright 2022 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.OverlaySheet {
    id: root;
    function disconnectGear(gearToDisconnect) {
        _private.needsUpdate = true;
        deviceID = gearToDisconnect;
        open();
    }
    property alias deviceID: relevantDeviceModel.filterString

    parent: Overlay.overlay
    showCloseButton: true;
    title: i18nc("Heading for the overlay for picking options when disconnecting from a piece of gear", "Disconnect?");

    ColumnLayout {
        Layout.fillWidth: true;
        spacing: Kirigami.Units.largeSpacing;
        Label {
            Layout.fillWidth: true;
            wrapMode: Text.Wrap;
            text: i18ncp("Label above the list of options available for disconnecting from a piece of gear", "Please select how you would like to disconnect from %2.", "Please select how you would like to disconnect from the %1 pieces of gear.", relevantDeviceModel.count, _private.firstDeviceName);
        }
        ColumnLayout {
            Layout.fillWidth: true;
            spacing: Kirigami.Units.smallSpacing
            visible: _private.hasNoPhoneMode
            Rectangle {
                Layout.fillWidth: true;
                Layout.minimumHeight: 1;
                Layout.maximumHeight: 1;
                color: Kirigami.Theme.disabledTextColor;
            }
            RowLayout {
                Label {
                    Layout.fillWidth: true;
                    text: i18nc("Label above the section that lets you set up the gear for use without a phone and then disconnect", "Use Gear Without Your Phone");
                    elide: Text.ElideRight
                }
                ToolButton {
                    Layout.minimumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.minimumHeight: Kirigami.Units.iconSizes.medium;
                    Layout.maximumHeight: Kirigami.Units.iconSizes.medium;
                    icon.name: "settings-configure";
                    onClicked: {
                        noPhoneModeSettings.visible = !noPhoneModeSettings.visible;
                    }
                }
                ToolButton {
                    Layout.minimumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.minimumHeight: Kirigami.Units.iconSizes.medium;
                    Layout.maximumHeight: Kirigami.Units.iconSizes.medium;
                    icon.name: "question";
                    onClicked: {
                        noPhoneModeDescription.visible = !noPhoneModeDescription.visible;
                    }
                }
            }
            Label {
                id: noPhoneModeDescription;
                visible: false;
                Layout.fillWidth: true;
                wrapMode: Text.Wrap;
                text: i18nc("Label above the list of options available for disconnecting from a piece of gear", "Pick this option to set up No Phone Mode and disconnect, so you can leave the phone off. This works like a basic Casual Mode, but without the phone to operate it. (Handy if you're costuming and have nowhere to stow your phone)");
            }
            ColumnLayout {
                id: noPhoneModeSettings
                visible: false
                Layout.fillWidth: true
                Repeater {
                    id: noPhoneModeGroupsRepeater
                    model: _private.noPhoneModeGroups
                    delegate: BasicListItem {
                        property bool categoryPicked: (model.index === 0)
                        property string categoryKey: model.index + 1
                        property string categoryValue: modelData
                        icon.source: categoryPicked ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
                        text: categoryValue
                        onClicked: {
                            categoryPicked = !categoryPicked;
                        }
                    }
                }
                Label {
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    text: i18nc("Label for the slider which lets the user pick how long to wait between No Phone mode picks its commands", "No Phone Mode will wait between %1 and %2 seconds before picking its next move.", Math.floor(pauseRangeSlider.first.value), Math.floor(pauseRangeSlider.second.value));
                }
                RangeSlider {
                    id: pauseRangeSlider;

                    first.value: Digitail.AppSettings.idleMinPause;
                    second.value: Digitail.AppSettings.idleMaxPause;
                    from: 15;
                    to: 240;
                    stepSize: 1.0;
                    Layout.leftMargin: Kirigami.Units.largeSpacing;
                    Layout.fillWidth: true;

                    Component.onCompleted: {
                        pauseRangeSlider.setValues(Digitail.AppSettings.idleMinPause, Digitail.AppSettings.idleMaxPause);
                    }
                }
                Label {
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    text: i18nc("Label for the slider which lets the user pick how long to wait before No Phone mode is fully activated", "No Phone Mode will wait %1 minute(s) before picking the first move.", noPhoneModeDelay.value);
                }
                Slider {
                    id: noPhoneModeDelay;
                    Layout.fillWidth: true;
                    from: 1;
                    to: 4;
                    stepSize: 1.0;
                }
            }
            Button {
                text: i18nc("Label for the button in the disconnection options popup which puts the gear into autonomous, or no phone, mode", "Engage No Phone Mode");
                Layout.fillWidth: true;
                onClicked: {
                    var constructedMessage = "AUTOMODE ";
                    if (noPhoneModeGroupsRepeater.count > 0) {
                        for (var i = 0; i < noPhoneModeGroupsRepeater.count; ++i) {
                            if (noPhoneModeGroupsRepeater.itemAt(i).categoryPicked) {
                                constructedMessage += "G" + noPhoneModeGroupsRepeater.itemAt(i).categoryKey;
                            }
                        }
                        constructedMessage += " ";
                    }
                    constructedMessage += "T" + Math.floor(pauseRangeSlider.first.value);
                    constructedMessage += "T" + Math.floor(pauseRangeSlider.second.value);
                    constructedMessage += "T25" + Math.floor(noPhoneModeDelay.value);
                    console.log("Sending no phone mode message: " + constructedMessage);
                    for (let deviceIndex = 0; deviceIndex < relevantDeviceModel.count; ++deviceIndex) {
                        let deviceID = relevantDeviceModel.data(relevantDeviceModel.index(deviceIndex, 0), Digitail.DeviceModelTypes.DeviceID);
                        Digitail.BTConnectionManager.sendMessage(constructedMessage, [deviceID]);
                        Digitail.BTConnectionManager.disconnectDevice(deviceID);
                    }
                    root.close();
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true;
            spacing: Kirigami.Units.smallSpacing
            Rectangle {
                Layout.fillWidth: true;
                Layout.minimumHeight: 1;
                Layout.maximumHeight: 1;
                color: Kirigami.Theme.disabledTextColor;
            }
            RowLayout {
                Label {
                    Layout.fillWidth: true;
                    text: i18nc("Label above the section that lets the user simply disconnect from the gear", "Just Disconnect");
                }
                ToolButton {
                    Layout.minimumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.minimumHeight: Kirigami.Units.iconSizes.medium;
                    Layout.maximumHeight: Kirigami.Units.iconSizes.medium;
                    icon.name: "question";
                    onClicked: {
                        justDisconnectDescription.visible = !justDisconnectDescription.visible;
                    }
                }
            }
            Label {
                id: justDisconnectDescription;
                visible: false;
                Layout.fillWidth: true;
                wrapMode: Text.Wrap;
                text: i18nc("Label above the list of options available for disconnecting from a piece of gear", "Pick this option to disconnect from the gear without turning it off (handy if you want to charge it).");
            }
            Button {
                text: i18nc("Label for the button in the disconnection options popup which only disconnects the gear", "Disconnect");
                Layout.fillWidth: true;
                onClicked: {
                    for (let deviceIndex = 0; deviceIndex < relevantDeviceModel.count; ++deviceIndex) {
                        let deviceID = relevantDeviceModel.data(relevantDeviceModel.index(deviceIndex, 0), Digitail.DeviceModelTypes.DeviceID);
                        Digitail.BTConnectionManager.disconnectDevice(deviceID);
                    }
                    root.close();
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true;
            spacing: Kirigami.Units.smallSpacing
            visible: _private.hasShutdown
            Rectangle {
                Layout.fillWidth: true;
                Layout.minimumHeight: 1;
                Layout.maximumHeight: 1;
                color: Kirigami.Theme.disabledTextColor;
            }
            RowLayout {
                Label {
                    Layout.fillWidth: true;
                    text: i18nc("Header label for the section which lets you tell the gear to shut down before disconnecting", "Disconnect and Shut Down");
                }
                ToolButton {
                    Layout.minimumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.minimumHeight: Kirigami.Units.iconSizes.medium;
                    Layout.maximumHeight: Kirigami.Units.iconSizes.medium;
                    icon.name: "question";
                    onClicked: {
                        shutdownDescription.visible = !shutdownDescription.visible;
                    }
                }
            }
            Label {
                id: shutdownDescription;
                visible: false;
                Layout.fillWidth: true;
                wrapMode: Text.Wrap;
                text: i18nc("Label above the list of options available for shutting down the piece of gear", "Pick this option to turn off the gear rather than simply disconnecting (handy if you're putting it away immediately)");
            }
            Button {
                text: i18nc("Label for the button in the disconnection options popup which only disconnects the gear", "Shut Down Gear");
                Layout.fillWidth: true;
                onClicked: {
                    for (let deviceIndex = 0; deviceIndex < relevantDeviceModel.count; ++deviceIndex) {
                        let deviceID = relevantDeviceModel.data(relevantDeviceModel.index(deviceIndex, 0), Digitail.DeviceModelTypes.DeviceID);
                        Digitail.BTConnectionManager.sendMessage("SHUTDOWN", [deviceID]);
                        Digitail.BTConnectionManager.disconnectDevice(deviceID);
                    }
                    root.close();
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: Kirigami.Units.largeSpacing
            QtObject {
                id: _private
                property bool hasNoPhoneMode: false
                property var noPhoneModeGroups: []
                property bool hasShutdown: false
                property string firstDeviceName: ""
                property bool needsUpdate: false;
            }
            Digitail.FilterProxyModel {
                id: deviceFilterProxy
                sourceModel: Digitail.DeviceModel;
                filterRole: Digitail.DeviceModelTypes.IsConnected;
                filterBoolean: true;
            }
            Digitail.FilterProxyModel {
                id: relevantDeviceModel
                sourceModel: deviceFilterProxy;
                filterRole: Digitail.DeviceModelTypes.DeviceID;
                onFilterStringChanged: updateData()
                onCountChanged: updateData()
                function updateData() {
                    dataUpdater.restart();
                }
            }
            Timer {
                id: dataUpdater
                interval: 2; running: false; repeat: false;
                onTriggered: {
                    if (_private.needsUpdate) {
                        let allHaveNPM = relevantDeviceModel.count > 0;
                        // Always assign the first set of groups, or we'll definitely be wrong...
                        let lowestCommonDenominatorNPMGroups = [];
                        let allHaveShutdown = relevantDeviceModel.count > 0;
                        for (let deviceIndex = 0; deviceIndex < relevantDeviceModel.count; ++deviceIndex) {
                            if (deviceIndex === 0) {
                                _private.firstDeviceName = relevantDeviceModel.data(relevantDeviceModel.index(0, 0), Digitail.DeviceModelTypes.Name);
                            }
                            if (relevantDeviceModel.data(relevantDeviceModel.index(deviceIndex, 0), Digitail.DeviceModelTypes.HasNoPhoneMode) == false) {
                                allHaveNPM = false;
                            }
                            let deviceNoPhoneModeGroups = relevantDeviceModel.data(relevantDeviceModel.index(deviceIndex, 0), Digitail.DeviceModelTypes.NoPhoneModeGroups);
                            if (deviceNoPhoneModeGroups == undefined) {
                                dataUpdater.restart();
                                return;
                            }
                            if (deviceIndex == 0 || lowestCommonDenominatorNPMGroups.length > deviceNoPhoneModeGroups.length) {
                                lowestCommonDenominatorNPMGroups = deviceNoPhoneModeGroups;
                            }
                        }
                        _private.hasNoPhoneMode = allHaveNPM;
                        _private.noPhoneModeGroups = lowestCommonDenominatorNPMGroups;
                        _private.hasShutdown = allHaveShutdown;
                        _private.needsUpdate = false;
                    }
                }
            }
        }
    }
}
