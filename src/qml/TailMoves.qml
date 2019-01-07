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

BaseMovesPage {
    categoriesModel: ListModel {
        ListElement {
            name: qsTr("Calm and Relaxed");
            category: "relaxed";
            color: "#a3d286";
        }
        ListElement {
            name: qsTr("Fast and Excited");
            category: "excited";
            color: "#00c0dc";
        }
        ListElement {
            name: qsTr("Frustrated and Tense");
            category: "tense";
            color: "#fb6b46";
        }
    }
}
