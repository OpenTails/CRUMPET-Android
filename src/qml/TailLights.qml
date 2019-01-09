/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import org.kde.kirigami 2.5 as Kirigami

BaseMovesPage {
    objectName: "tailLights";
    actions {
        main: Kirigami.Action {
            text: qsTr("Stop Lights");
            icon.name: ":/org/kde/kirigami/icons/dialog-cancel.svg";
            onTriggered: {
                connectionManager.sendMessage("LEDOFF");
            }
        }
    }
    categoriesModel: ListModel {
        ListElement {
            name: qsTr("LED Patterns");
            category: "lights";
            color: "#93cee9";
        }
    }
}
