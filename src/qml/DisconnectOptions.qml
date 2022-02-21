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

    header: Kirigami.Heading {
        text: i18nc("Heading for the overlay for picking options when disconnecting from a piece of gear", "Disconnect?");
        wrapMode: Text.Wrap;
    }
    ColumnLayout {
        Layout.fillWidth: true;
        Layout.fillHeight: true;
        spacing: 0
        Repeater {
            model: Digitail.FilterProxyModel {
                id: relevantDeviceModel
                sourceModel: DeviceModel;
                filterRole: 258 // The deviceID role
            }
            delegate: ColumnLayout {
                Layout.fillWidth: true;
                Layout.fillHeight: true;
                spacing: Kirigami.Units.largeSpacing;
                Label {
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    text: i18nc("Label above the list of options available for disconnecting from a piece of gear", "Please select how you would like to disconnect from %1.", model.name);
                }
                ColumnLayout {
                    Layout.fillWidth: true;
                    spacing: Kirigami.Units.smallSpacing
                    visible: model.hasNoPhoneMode
                    Rectangle {
                        Layout.fillWidth: true;
                        Layout.minimumHeight: 1
                        Layout.maximumHeight: 1
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Label {
                        Layout.fillWidth: true;
                        wrapMode: Text.Wrap;
                        text: i18nc("Label above the list of options available for disconnecting from a piece of gear", "Pick this option to set up No Phone Mode and disconnect, so you can leave the phone off. This works like a basic Casual Mode, but without the phone to operate it. (Handy if you're costuming and have nowhere to stow your phone)");
                    }
                    Repeater {
                        id: noPhoneModeCategoriesRepeater
                        model: Object.keys(model.noPhoneModeCategories).length
                        delegate: Kirigami.BasicListItem {
                            property bool categoryPicked: (model.index === 0)
                            property string categoryKey: Object.keys(model.noPhoneModeCategories)[idx]
                            property string categoryValue: model.noPhoneModeCategories[key]
                            icon: categoryPicked ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                            label: categoryValue
                            onClicked: {
                                categoryPicked = !categoryPicked;
                            }
                        }
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

                        first.onMoved: {
                            AppSettings.idleMinPause = first.value;
                        }

                        second.onMoved: {
                            if (second.value < 20) {
                                second.value = 20;
                            }
                            AppSettings.idleMaxPause = second.value;
                        }

                        Component.onCompleted: {
                            pauseRangeSlider.setValues(AppSettings.idleMinPause, AppSettings.idleMaxPause);
                        }
                    }

                    Item {
                        Layout.minimumHeight: childrenRect.height;
                        Layout.maximumHeight: childrenRect.height;
                        Layout.leftMargin: Kirigami.Units.largeSpacing;
                        Layout.fillWidth: true;
                        Label {
                            text: Math.floor(pauseRangeSlider.first.value);
                            anchors {
                                left: parent.left;
                                right: parent.horizontalCentre;
                            }
                        }
                        Label {
                            text: Math.floor(pauseRangeSlider.second.value);
                            anchors {
                                left: parent.horizontalCentre;
                                right: parent.right;
                            }
                        }
                    }
                    Button {
                        text: i18nc("Label for the button in the disconnection options popup which puts the gear into autonomous, or no phone, mode", "Engage No Phone Mode");
                        onClicked: {
                            var constructedMessage = "AUTOMODE ";
                            if (noPhoneModeCategoriesRepeater.count > 0) {
                                for (var i = 0; i < noPhoneModeCategoriesRepeater.count; ++i) {
                                    constructedMessage += "G" + noPhoneModeCategoriesRepeater.item(i).key;
                                }
                                constructedMessage += " ";
                            }
                            constructedMessage += "T" + pauseRangeSlider.first.value;
                            constructedMessage += "T" + pauseRangeSlider.second.value;
                            constructedMessage += "T254";
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
                        Layout.minimumHeight: 1
                        Layout.maximumHeight: 1
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Label {
                        Layout.fillWidth: true;
                        wrapMode: Text.Wrap;
                        text: i18nc("Label above the list of options available for disconnecting from a piece of gear", "Pick this option to disconnect from the gear without turning it off (handy if you want to charge it).");
                    }
                    Button {
                        text: i18nc("Label for the button in the disconnection options popup which only disconnects the gear", "Disconnect");
                        onClicked: {
                            BTConnectionManager.disconnectDevice(model.deviceID);
                            root.close();
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true;
                    spacing: Kirigami.Units.smallSpacing
                    visible: model.hasShutdown
                    Rectangle {
                        Layout.fillWidth: true;
                        Layout.minimumHeight: 1
                        Layout.maximumHeight: 1
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Label {
                        Layout.fillWidth: true;
                        wrapMode: Text.Wrap;
                        text: i18nc("Label above the list of options available for shutting down the piece of gear", "Pick this option to turn off the gear rather than simply disconnecting (handy if you're putting it away immediately)");
                    }
                    Button {
                        text: i18nc("Label for the button in the disconnection options popup which only disconnects the gear", "Shut Down Gear");
                        onClicked: {
                            BTConnectionManager.sendMessage("SHUTDOWN", [model.deviceID]);
                            BTConnectionManager.disconnectDevice(model.deviceID);
                            root.close();
                        }
                    }
                }
            }
        }
    }
}
