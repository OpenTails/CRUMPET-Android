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

Kirigami.ScrollablePage {
    id: root;
    objectName: "settingsPage";
    title: qsTr("Settings");

    ColumnLayout {
        width: parent.width;
        spacing: Kirigami.Units.largeSpacing;

        SettingsCard {
            headerText: qsTr("Instructions");
            descriptionText: qsTr("Please download and read the instructions by clicking the link below. This includes instructions on how to wear your tail or ears, along with some nice graphics showing you how.");
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
            headerText: qsTr("Automatic Reconnection");
            descriptionText: qsTr("In certain situations, the app might lose its connection to your gear. Ticking this option will ensure that the app will attempt to reconnect automatically when the connection is lost.");
            footer: QQC2.CheckBox {
                text: qsTr("Reconnect Automatically");
                checked: AppSettings.autoReconnect;
                onClicked: {
                    AppSettings.autoReconnect = !AppSettings.autoReconnect;
                }
            }
        }

        SettingsCard {
            headerText: qsTr("Gear Names");
            descriptionText: qsTr("If you want to clear the names of any gear you have given a name, click the button below to make the app forget them all.");
            footer: QQC2.Button {
                text: qsTr("Forget Gear Names")
                Layout.fillWidth: true
                onClicked: {
                    showMessageBox(qsTr("Clear the names?"),
                        qsTr("Please confirm that you do, in fact, want to clear all your saved device names."),
                        function () { BTConnectionManager.clearDeviceNames(); });
                }
            }
        }

        SettingsCard {
            headerText: qsTr("Gear Commands");
            descriptionText: qsTr("While the app ships with a number of command sets out of the box, this is not all that your gear can do. You can modify what is sent to your gear, and we'll remember for each of them which commands you want to use. To use this functionality, give your gear a name, and you'll be able to change what commands it will use here.");
            footer: ColumnLayout {
                Repeater {
                    model: Object.keys(AppSettings.deviceNames);
                    delegate: Kirigami.BasicListItem {
                        Layout.fillWidth: true;
                        icon: ":/images/moves.svg";
                        separatorVisible: false;
                        label: AppSettings.deviceNames[modelData];
                        onClicked: {
                            pageStack.push(gearCommandsPage);
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
            headerText: qsTr("Fake Tail");
            descriptionText: qsTr("If you have just downloaded the app, for example in anticipation of the arrival of your brand new, super shiny DIGITAiL or EarGear, you might want to explore what the app can do. You can click the button below to trick the app into thinking that there is a tail nearby, and let you explore what options exist. Enabling this option will make a fake tail show up on the welcome page.");
            footer: QQC2.CheckBox {
                text: qsTr("Show Fake Tail")
                Layout.fillWidth: true
                checked: AppSettings.fakeTailMode;

                onClicked: {
                    AppSettings.fakeTailMode = !AppSettings.fakeTailMode;
                }
            }
        }
    }
}
