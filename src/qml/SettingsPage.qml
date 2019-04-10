/*
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
import QtQuick.Controls 2.4 as QQC2
import org.kde.kirigami 2.6 as Kirigami
import QtQuick.Layouts 1.11

Kirigami.ScrollablePage {
    objectName: "settingsPage";
    title: qsTr("Settings");

    ColumnLayout {
        width: parent.width;

        Kirigami.AbstractCard {
            Layout.fillWidth: true

            header: Kirigami.Heading {
                text: qsTr("Use Auto-reconnect if your tail drops the connection?");
                QQC2.CheckBox {
                    checked: AppSettings.autoReconnect;

                    height: parent.height;
                    width: height;
                    anchors.right: parent.right;

                    onClicked: {
                        AppSettings.autoReconnect = !AppSettings.autoReconnect;
                    }
                }
            }
        }

        QQC2.Button {
            text: qsTr("Fake it!")
            Layout.fillWidth: true

            onClicked: {
                BTConnectionManager.stopDiscovery();
                BTConnectionManager.setFakeTailMode(true);
            }
        }
    }
}
