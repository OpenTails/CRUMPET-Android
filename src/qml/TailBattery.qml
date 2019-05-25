/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.4 as Kirigami
import org.thetailcompany.digitail 1.0

Item {
    visible: height > 0;
    height: BTConnectionManager.isConnected ? batteryLabel.height : 0;

    Label {
        id: batteryLabel;
        text: "Tail battery:"
        height: Kirigami.Units.iconSizes.small;
    }

    Row {
        spacing: Kirigami.Units.smallSpacing;
        anchors.right: parent.right;

        Repeater {
            model: 4;

            Rectangle {
                height: Kirigami.Units.iconSizes.small;
                width: height;
                radius: height / 2;
                color: "transparent";

                border {
                    width: 1;
                    color: BTConnectionManager.batteryLevel <= 1 ? "red" : "black";
                }

                anchors.verticalCenter: parent.verticalCenter;

                Rectangle {
                    visible: modelData < BTConnectionManager.batteryLevel;
                    height: parent.height - Kirigami.Units.smallSpacing * 2;
                    width: height;
                    radius: height / 2;
                    color: BTConnectionManager.batteryLevel <= 1 ? "red" : "black";
                    anchors.centerIn: parent;
                }
            }
        }
    }
}
