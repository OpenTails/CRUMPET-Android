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

import QtQuick 2.7
import QtQuick.Controls 2.11
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.OverlaySheet {
    id: root;
    function disconnectGear(gearToDisconnect) {
        deviceID = gearToDisconnect;
        open();
    }
    property alias deviceID: relevantDeviceModel.filterString

    showCloseButton: true;
    title: i18nc("Heading for the overlay for picking options when disconnecting from a piece of gear", "Disconnect?");
    contentItem: ColumnLayout {
        Layout.fillWidth: true;
        spacing: 0
        Repeater {
            model: Digitail.FilterProxyModel {
                id: relevantDeviceModel
                sourceModel: DeviceModel;
                filterRole: 258 // The deviceID role
            }
            delegate: ColumnLayout {
                id: disconnectOptionsDelegate
                Layout.fillWidth: true;
                Layout.fillHeight: true;
                spacing: Kirigami.Units.largeSpacing;
                property var funnyGroups: model.noPhoneModeGroups ? model.noPhoneModeGroups : []
                Label {
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    text: i18nc("Label above the list of options available for disconnecting from a piece of gear", "Please select how you would like to disconnect from %1.", (model.name ? model.name : " "));
                }
                ColumnLayout {
                    Layout.fillWidth: true;
                    spacing: Kirigami.Units.smallSpacing
                    visible: model.hasNoPhoneMode ? model.hasNoPhoneMode : false
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
                    Repeater {
                        id: noPhoneModeGroupsRepeater
                        model: disconnectOptionsDelegate.funnyGroups
                        delegate: Kirigami.BasicListItem {
                            property bool categoryPicked: (model.index === 0)
                            property string categoryKey: model.index + 1
                            property string categoryValue: modelData
                            icon: categoryPicked ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                            label: categoryValue
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

                        first.value: AppSettings.idleMinPause;
                        second.value: AppSettings.idleMaxPause;
                        from: 15;
                        to: 240;
                        stepSize: 1.0;
                        Layout.leftMargin: Kirigami.Units.largeSpacing;
                        Layout.fillWidth: true;

                        Component.onCompleted: {
                            pauseRangeSlider.setValues(AppSettings.idleMinPause, AppSettings.idleMaxPause);
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
                    Button {
                        text: i18nc("Label for the button in the disconnection options popup which puts the gear into autonomous, or no phone, mode", "Engage No Phone Mode");
                        Layout.fillWidth: true;
                        onClicked: {
                            var constructedMessage = "AUTOMOVE ";
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
                            BTConnectionManager.sendMessage(constructedMessage, [model.deviceID]);
                            BTConnectionManager.disconnectDevice(model.deviceID);
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
                            BTConnectionManager.disconnectDevice(model.deviceID);
                            root.close();
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true;
                    spacing: Kirigami.Units.smallSpacing
                    visible: model.hasShutdown ? model.hasShutdown : false
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
                            BTConnectionManager.sendMessage("SHUTDOWN", [model.deviceID]);
                            BTConnectionManager.disconnectDevice(model.deviceID);
                            root.close();
                        }
                    }
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: 150
        }
    }
}
