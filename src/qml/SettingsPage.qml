/*
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Controls 2.4 as QQC2
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Layouts 1.11
import org.thetailcompany.digitail 1.0

Kirigami.ScrollablePage {
    id: root;
    objectName: "settingsPage";
    title: i18nc("Title for the settings page", "Settings");

    ColumnLayout {
        width: parent.width;
        spacing: Kirigami.Units.largeSpacing;

        SettingsCard {
            headerText: i18nc("Header for the instruction panel, on the settings page", "Instructions");
            descriptionText: i18nc("Description for the instruction panel on the settings page", "Please download and read the instructions by clicking the link below. This includes instructions on how to wear your tail or ears, along with some nice graphics showing you how.");
            footer: ColumnLayout{
                Layout.fillWidth: true; Layout.fillHeight: true;
                    Kirigami.UrlButton {
                        Layout.fillWidth: true; Layout.fillHeight: true;
                        url: "http://thetailcompany.com/digitail.pdf";
                    }
                    Kirigami.UrlButton {
                        Layout.fillWidth: true; Layout.fillHeight: true;
                        url: "http://thetailcompany.com/eargear.pdf";
                    }
            }
        }

        SettingsCard {
            headerText: i18nc("Header for the panel on automatic reconnection, on the settings page", "Automatic Reconnection");
            descriptionText: i18nc("Description for the panel on automatic reconnection, on the settings page", "In certain situations, the app might lose its connection to your gear. Ticking this option will ensure that the app will attempt to reconnect automatically when the connection is lost.");
            footer: QQC2.CheckBox {
                text: i18nc("Checkbox for enabling automatic reconnection, on the automatic reconnection panel in the settings page", "Reconnect Automatically");
                checked: AppSettings.autoReconnect;
                onClicked: {
                    AppSettings.autoReconnect = !AppSettings.autoReconnect;
                }
            }
        }

        SettingsCard {
            headerText: i18nc("Header for the panel for changing gear names, on the settings page", "Gear Names");
            descriptionText: i18nc("Description for the panel for changing gear names, on the settings page", "If you want to clear the names of any gear you have given a name, click the button below to make the app forget them all.");
            footer: QQC2.Button {
                text: i18nc("Label for the button for clearing gear names, on the panel for changing gear names, on the settings page", "Forget Gear Names")
                Layout.fillWidth: true
                onClicked: {
                    showMessageBox(i18nc("Header for the warning prompt for clearing gear names, on the panel for changing gear names, on the settings page", "Clear the names?"),
                        i18nc("Text for the warning prompt for clearing gear names, on the panel for changing gear names, on the settings page", "Please confirm that you do, in fact, want to clear all your saved device names."),
                        function () { BTConnectionManager.clearDeviceNames(); });
                }
            }
        }

        SettingsCard {
            headerText: i18nc("Header for the panel for adding commands to your gear, on the settings page", "Gear Commands");
            descriptionText: i18nc("Description for the panel for adding commands to your gear, on the settings page", "While the app ships with a number of command sets out of the box, this is not all that your gear can do. You can modify what is sent to your gear, and we'll remember for each of them which commands you want to use. To be able to more easily use this, make sure to give your gear a name, so you can tell them apart more easily.");
            footer: ColumnLayout {
                QQC2.CheckBox {
                    id: showOnlyConnectedGear;
                    text: i18nc("Checkbox for hiding non-connected gear, on the panel for adding commands to your gear, on the settings page", "Only Show Connected Gear");
                    FilterProxyModel {
                        id: onlyConnectedFilterProxy;
                        sourceModel: DeviceModel;
                        filterRole: 262; // the isConnected role
                        filterBoolean: true;
                    }
                }
                Kirigami.BasicListItem {
                    visible: gearCommandsRepeater.count === 0;
                    Layout.fillWidth: true;
                    icon: ":/images/moves.svg";
                    separatorVisible: false;
                    label: i18nc("Warning for the list of configurable gear being empty, on the panel for adding commands to your gear, on the settings page", "No gear to configure...");
                    enabled: false;
                }
                Repeater {
                    id: gearCommandsRepeater;
                    model: showOnlyConnectedGear.checked ? onlyConnectedFilterProxy : DeviceModel;
                    delegate: Kirigami.BasicListItem {
                        Layout.fillWidth: true;
                        icon: ":/images/moves.svg";
                        separatorVisible: false;
                        label: model.name;
                        onClicked: {
                            pageStack.push(gearCommandsPage, { deviceID: model.deviceID });
                        }
                    }
                }
            }
            Component {
                id: gearCommandsPage;
                SettingsCrumpetPicker {}
            }
        }

        SettingsCard {
            visible: gearCommandsRepeaterEarOptions.count > 0;
            headerText: i18nc("Heading for the panel for adjusting microphone levels for the earGear, on the settings page", "EarGear Hearing");
            descriptionText: i18nc("Description for the panel for adjusting microphone levels for the earGear, on the settings page", "If your EarGear is acting a bit like it isn't hearing right, it might need to have its balance checked. To make sure it works the best, you will want to be in a quiet room before you tap the balance button below (pause your music, close the door to your bedroom, etc). This value is stored in the box, so you shouldn't generally need to do it more than once. You might also need to swap the direction around (for example, you might want to put your controller box either on the front or the back of your body, or upside down so the cable can run more gracefully).");
            footer: ColumnLayout {
                Repeater {
                    id: gearCommandsRepeaterEarOptions;
                    model: FilterProxyModel {
                        sourceModel: onlyConnectedFilterProxy;
                        filterRole: 265; // the hasListening role
                        filterBoolean: true;
                    }
                    delegate: ColumnLayout {
                        Kirigami.Heading {
                            Layout.fillWidth: true;
                            text: model.name;
                        }
                        QQC2.Button {
                            text: model.micsSwapped === true ?  
                                i18nc("Label for the button for unswapping left and right, on the panel for adjusting microphone levels for the earGear, on the settings page", "Unswap left and right") : 
                                i18nc("Label for the button for swapping left and right, the panel for adjusting microphone levels for the earGear, on the settings page", "Swap left and right");
                            Layout.fillWidth: true;
                            onClicked: {
                                BTConnectionManager.sendMessage("MICSWAP", [model.deviceID]);
                            }
                        }
                        QQC2.Button {
                            text: i18nc("Label for the button for rebalancing microphone levels, the panel for adjusting microphone levels for the earGear, on the settings page", "Rebalance Microphones");
                            Layout.fillWidth: true;
                            onClicked: {
                                BTConnectionManager.sendMessage("MICBAL", [model.deviceID]);
                            }
                        }
                    }
                }
            }
        }

        SettingsCard {
            headerText: i18nc("Heading for the panel for the demo modus of the app, on the settings page","Fake Tail");
            descriptionText: i18nc("Description for the panel for the demo modus of the app, on the settings page", "If you have just downloaded the app, for example in anticipation of the arrival of your brand new, super shiny DIGITAiL or EarGear, you might want to explore what the app can do. You can click the button below to trick the app into thinking that there is a tail nearby, and let you explore what options exist. Enabling this option will make a fake tail show up on the welcome page.");
            footer: QQC2.CheckBox {
                text: i18nc("Label for the checkbox for showing a fake tail inside the app, on the panel for the demo modus of the app, on the settings page", "Show Fake Tail")
                Layout.fillWidth: true
                checked: AppSettings.fakeTailMode;

                onClicked: {
                    AppSettings.fakeTailMode = !AppSettings.fakeTailMode;
                }
            }
        }
    }
}
