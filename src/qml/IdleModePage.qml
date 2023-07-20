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
import QtQuick.Controls 2.4
import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    objectName: "idleMode";
    title: i18nc("Title for the page for selecting a move in Casual Mode", "Casual Mode Categories");

    Component {
        id: idleCategoryDelegate;
        Kirigami.BasicListItem {
            icon: Digitail.AppSettings.idleCategories.indexOf(modelData["category"]) >= 0 ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
            label: modelData["name"];
            onClicked: {
                var idx = Digitail.AppSettings.idleCategories.indexOf(modelData["category"]);
                if(idx >= 0) {
                    Digitail.AppSettings.removeIdleCategory(modelData["category"]);
                }
                else {
                    Digitail.AppSettings.addIdleCategory(modelData["category"]);
                }
            }
        }
    }
    property var availableCategories: [
        {
            name: i18nc("Description for the category for the Relaxed Moveset, on the page for selecting a move in Casual Mode", "Calm and Relaxed"),
            category: "relaxed",
            color: "#1cdc9a",
        },
        {
            name: i18nc("Description for the category for the Excited Moveset, on the page for selecting a move in Casual Mode", "Fast and Excited"),
            category: "excited",
            color: "#c9ce3b",
        },
        {
            name: i18nc("Description for the category for the Tense Moveset, on the page for selecting a move in Casual Mode", "Frustrated and Tense"),
            category: "tense",
            color: "#f67400",
        },
        {
            name: i18nc("Description for the category for the LED Patterns, on the page for selecting a move in Casual Mode", "LED Patterns"),
            category: "lights",
            color: "#93cee9",
        }
    ]

    ListView {
        model: availableCategories;
        delegate: idleCategoryDelegate;

        header: InfoCard {
            text: i18nc("Infocard for selecting the pauses in between moves, on the page for selecting a move in Casual Mode", "This is where you set the pauses in between moves, as well as the families of moves that can be called upon.");
        }
        footer: Kirigami.AbstractCard {
            contentItem: IdlePauseRangePicker {
            }

            anchors {
                left: parent.left;
                right: parent.right;
            }
        }
    }
}

