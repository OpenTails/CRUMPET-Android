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

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.ScrollablePage {
    objectName: "idleMode";
    title: i18nc("Title for the Casual Mode settings page", "Casual Mode Categories");

    Component {
        id: idleCategoryDelegate;
        BasicListItem {
            icon.source: Digitail.AppSettings.idleCategories.indexOf(modelData["category"]) >= 0 ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
            text: modelData["name"];
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
            name: i18nc("Description for the category for the Relaxed Moveset, on the Casual Mode settings page", "Calm and Relaxed"),
            category: "relaxed",
            color: "#1cdc9a",
        },
        {
            name: i18nc("Description for the category for the Excited Moveset, on the Casual Mode settings page", "Fast and Excited"),
            category: "excited",
            color: "#c9ce3b",
        },
        {
            name: i18nc("Description for the category for the Tense Moveset, on the Casual Mode settings page", "Frustrated and Tense"),
            category: "tense",
            color: "#f67400",
        },
        {
            name: i18nc("Description for the category for the LED Patterns, on the Casual Mode settings page", "LED Patterns"),
            category: "lights",
            color: "#93cee9",
        }
    ]

    ListView {
        model: availableCategories;
        delegate: idleCategoryDelegate;

        header: InfoCard {
            text: i18nc("Infocard for selecting the pauses in between moves, on the Casual Mode settings page", "This is where you set the pauses in between moves, as well as the families of moves that can be called upon.");
            footer: BasicListItem {
                text: i18nc("Label for the button for enabling the Casual Mode, on the Casual Mode settings page", "Enable Casual Mode");
                icon.source: (Digitail.AppSettings !== null && Digitail.AppSettings.idleMode) ? "qrc:/icons/breeze-internal/emblems/16/checkbox-checked" : "qrc:/icons/breeze-internal/emblems/16/checkbox-unchecked";
                onClicked: { Digitail.AppSettings.idleMode = !Digitail.AppSettings.idleMode; }
            }
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
