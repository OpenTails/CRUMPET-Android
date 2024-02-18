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

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs as QtDialogs
import QtQuick.Layouts
import org.thetailcompany.digitail as Digitail

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
                    QQC2.Label {
                        Layout.fillWidth: true; Layout.fillHeight: true;
                        horizontalAlignment: Text.AlignHCenter
                        text: i18nc("Label showing all the items underneath are links to a PDF file", "Download PDF:")
                    }
                    Kirigami.UrlButton {
                        Layout.fillWidth: true; Layout.fillHeight: true;
                        text: i18nc("Label for downloading the general guide for safe use of the various gear", "Responsible Wagging Guide");
                        url: "http://thetailcompany.com/responsiblewagging.pdf";
                    }
                    Kirigami.UrlButton {
                        Layout.fillWidth: true; Layout.fillHeight: true;
                        text: i18nc("Label for downloading the instructions for the MiTail", "MiTail Instructions");
                        url: "http://thetailcompany.com/mitail.pdf";
                    }
                    Kirigami.UrlButton {
                        Layout.fillWidth: true; Layout.fillHeight: true;
                        text: i18nc("Label for downloading the instructions for the Digitail", "Digitail Instructions");
                        url: "http://thetailcompany.com/digitail.pdf";
                    }
                    Kirigami.UrlButton {
                        Layout.fillWidth: true; Layout.fillHeight: true;
                        text: i18nc("Label for downloading the instructions for EarGear", "EarGear Instructions");
                        url: "http://thetailcompany.com/eargear.pdf";
                    }
            }
        }

        SettingsCard {
            headerText: i18nc("Header for the panel on automatic reconnection, on the settings page", "Automatic Reconnection");
            descriptionText: i18nc("Description for the panel on automatic reconnection, on the settings page", "In certain situations, the app might lose its connection to your gear. Ticking this option will ensure that the app will attempt to reconnect automatically when the connection is lost.");
            footer: QQC2.CheckBox {
                text: i18nc("Checkbox for enabling automatic reconnection, on the automatic reconnection panel in the settings page", "Reconnect Automatically");
                checked: Digitail.AppSettings.autoReconnect;
                onClicked: {
                    Digitail.AppSettings.autoReconnect = !Digitail.AppSettings.autoReconnect;
                }
            }
        }

        SettingsCard {
            headerText: i18nc("Header for the panel for whether or not to always send to all connected devices, on the settings page", "Send To All");
            descriptionText: i18nc("Description for the panel for whether or not to always send to all connected devices, on the settings page", "When picking something to send to your gear, and you have more than one connected at the same time, you might want to always send to all of your gear, or not. Check the box here to make that choice, or to undo the choice made in the dialogue itself.");
            footer: QQC2.CheckBox {
                text: i18nc("Checkbox for the option to always send any selected command to all connected devices, on the panel for whether or not to always send to all connected devices in the settings page", "Always Send To All");
                checked: Digitail.AppSettings.alwaysSendToAll;
                onClicked: {
                    Digitail.AppSettings.alwaysSendToAll = !Digitail.AppSettings.alwaysSendToAll;
                }
            }
        }

        SettingsCard {
            headerText: i18nc("Header for the panel for changing gear names, on the settings page", "Gear Names");
            descriptionText: i18nc("Description for the panel for changing gear names, on the settings page", "If you want to clear the names of any gear you have given a name, click the button below to make the app forget them all. If you just want to rename a piece of gear, you can do that by tapping on the gear's name in the list over on the home page.");
            footer: QQC2.Button {
                text: i18nc("Label for the button for clearing gear names, on the panel for changing gear names, on the settings page", "Forget Gear Names")
                Layout.fillWidth: true
                onClicked: {
                    showMessageBox(i18nc("Header for the warning prompt for clearing gear names, on the panel for changing gear names, on the settings page", "Clear the names?"),
                        i18nc("Text for the warning prompt for clearing gear names, on the panel for changing gear names, on the settings page", "Please confirm that you do, in fact, want to clear all your saved device names."),
                        function () { Digitail.BTConnectionManager.clearDeviceNames(); });
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
                    Digitail.FilterProxyModel {
                        id: onlyConnectedFilterProxy;
                        sourceModel: Digitail.DeviceModel;
                        filterRole: Digitail.DeviceModelTypes.IsConnected;
                        filterBoolean: true;
                    }
                }
                BasicListItem {
                    visible: gearCommandsRepeater.count === 0;
                    Layout.fillWidth: true;
                    icon.source: "qrc:/images/moves.svg";
                    text: i18nc("Warning for the list of configurable gear being empty, on the panel for adding commands to your gear, on the settings page", "No gear to configure...");
                    enabled: false;
                }
                Repeater {
                    id: gearCommandsRepeater;
                    model: showOnlyConnectedGear.checked ? onlyConnectedFilterProxy : Digitail.DeviceModel;
                    delegate: BasicListItem {
                        Layout.fillWidth: true;
                        icon.source: "qrc:/images/moves.svg";
                        text: model.name;
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
                    model: Digitail.FilterProxyModel {
                        sourceModel: onlyConnectedFilterProxy;
                        filterRole: Digitail.DeviceModelTypes.CanBalanceListening;
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
                                Digitail.BTConnectionManager.sendMessage("MICSWAP", [model.deviceID]);
                            }
                        }
                        QQC2.Button {
                            text: i18nc("Label for the button for rebalancing microphone levels, the panel for adjusting microphone levels for the earGear, on the settings page", "Rebalance Microphones");
                            Layout.fillWidth: true;
                            onClicked: {
                                Digitail.BTConnectionManager.sendMessage("MICBAL", [model.deviceID]);
                            }
                        }
                    }
                }
            }
        }

        SettingsCard {
            visible: otaRepearer.count > 0
            headerText: i18nc("Heading for the panel for checking for and performing firmware updates for gear which supports this", "Gear Firmware");
            descriptionText: i18nc("Description for the panel for checking for and perfirming firmware updates for gear which supports this", "If your gear supports firmware updates, you can check for new ones by clicking the \"Check\" button below, and if there is one, you can then click the button underneath to download and install the update.");
            footer: ColumnLayout {
                Repeater {
                    id: otaRepearer
                    model: Digitail.FilterProxyModel {
                        sourceModel: onlyConnectedFilterProxy
                        filterRole: Digitail.DeviceModelTypes.SupportsOTA;
                        filterBoolean: true;
                    }
                    delegate: ColumnLayout {
                        Kirigami.Heading {
                            Layout.fillWidth: true;
                            text: model.name
                        }
                        QQC2.Button {
                            Layout.fillWidth: true;
                            text: i18nc("Label for the button which makes the app go online and check for the newest firmware available for the specific device", "Check For New Firmware");
                            onClicked: {
                                Digitail.BTConnectionManager.callDeviceFunction(model.deviceID, "checkOTA");
                            }
                        }
                        QQC2.Label {
                            visible: model.hasAvailableOTA === true && model.hasOTAData === false
                            Layout.fillWidth: true;
                            horizontalAlignment: Text.AlignHCenter;
                            text: i18nc("Label displaying the currently installed and available firmware versions when they are different", "You currently have version %1 installed, and version %2 is available for you to install", model.deviceVersion ? model.deviceVersion : " ", model.otaVersion ? model.otaVersion : " ")
                            wrapMode: Text.Wrap;
                        }
                        QQC2.Button {
                            visible: model.hasAvailableOTA === true && model.hasOTAData === false
                            Layout.fillWidth: true;
                            text: i18nc("Label for the button which makes the app download the newest available firmware (only visible when updated firmware has been found)", "Download");
                            onClicked: {
                                Digitail.BTConnectionManager.callDeviceFunction(model.deviceID, "downloadOTAData");
                            }
                        }
                        QQC2.Button {
                            visible: Digitail.AppSettings.developerMode === true && model.hasOTAData === false
                            Layout.fillWidth: true;
                            text: i18nc("Label for a button which allows the user to pick a file to load the firmware from (this is only available in developer mode)", "Load From File");
                            onClicked: {
                                firmwareFilePicker.open();
                            }
                            QtDialogs.FileDialog {
                                id: firmwareFilePicker
                                title: i18nc("Title for a dialog which allows the user to pick a file to load as firmware (this is only available in developer mode)", "Pick Firmware File")
                                nameFilters: ["Firmware Files (*.bin)", "All Files (*)"]
                                selectedNameFilter.index: 0
                                currentFolder: shortcuts.documents
                                onAccepted: {
                                    Digitail.BTConnectionManager.callDeviceFunctionWithParameter(model.deviceID, "loadFirmwareFile", fileUrl.toString());
                                }
                            }
                        }
                        QQC2.Label {
                            visible: model.hasOTAData === true
                            Layout.fillWidth: true;
                            text: i18nc("Label giving a set of instructions and warnings that users should be aware of before attempting to install new firmware", "Before updating your gear from %1 to %2, please make sure you do the following:\n\n- Connect your gear to the USB charger and make sure it is charging. Keep it connected to the power while you update the firmware.\n\n- Also make sure your phone has more than 50% charge before you begin the update process.\n\nUpgrading your gear takes around 3 minutes. You must not switch off the gear or the phone while its working!\n\nOnce the new firmware is beamed to your gear, it will disconnect from the app and reboot. This is quite normal! It should automatically reconnect, but in case it doesn't, just click the Connect button on the front page as usual.", model.deviceVersion ? model.deviceVersion : " ", model.otaVersion ? model.otaVersion : " ")
                            wrapMode: Text.Wrap;
                        }
                        QQC2.Button {
                            visible: model.hasOTAData === true
                            Layout.fillWidth: true;
                            text: i18nc("Label for the button which makes the app install the newest available firmware (only visible when updated firmware has been downloaded successfully)", "Install");
                            onClicked: {
                                Digitail.BTConnectionManager.callDeviceFunction(model.deviceID, "startOTA");
                            }
                        }
                    }
                }
            }
        }

        SettingsCard {
            headerText: i18nc("Heading for the panel of the language selector, on the settings page", "Language");
            descriptionText: i18nc("Description of the language selection option on the settings page", "Usually, Crumpet will use the language that your device uses, or English if that language is not available. However, if you wish to force some language, you can select it here, and we will use that language instead.");
            footer: QQC2.ComboBox {
                model: Digitail.AppSettings.availableLanguages
                currentIndex: {
                    var found = 0; // Default language option
                    for (var i = 0; i < count; ++i) {
                        if (Digitail.AppSettings.availableLanguages[i].endsWith("(%1)".arg(Digitail.AppSettings.languageOverride))) {
                            found = i;
                            break;
                        }
                    }
                    return found;
                }
                onActivated: {
                    if (currentIndex == 0) {
                        Digitail.AppSettings.languageOverride = "";
                    } else {
                        Digitail.AppSettings.languageOverride = Digitail.AppSettings.availableLanguages[currentIndex];
                    }
                }
            }
        }

        SettingsCard {
            headerText: i18nc("Heading for the panel for the demo modus of the app, on the settings page", "Fake Tail");
            descriptionText: i18nc("Description for the panel for the demo modus of the app, on the settings page", "If you have just downloaded the app, for example in anticipation of the arrival of your brand new, super shiny MiTail or EarGear, you might want to explore what the app can do. You can click the button below to trick the app into thinking that there is a tail nearby, and let you explore what options exist. Enabling this option will make a fake tail show up on the welcome page.");
            footer: QQC2.CheckBox {
                text: i18nc("Label for the checkbox for showing a fake tail inside the app, on the panel for the demo modus of the app, on the settings page", "Show Fake Tail")
                Layout.fillWidth: true
                checked: Digitail.AppSettings.fakeTailMode;

                onClicked: {
                    Digitail.AppSettings.fakeTailMode = !Digitail.AppSettings.fakeTailMode;
                }
            }
        }
    }
}
