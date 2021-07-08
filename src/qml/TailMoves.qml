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
import org.kde.kirigami 2.13 as Kirigami

Kirigami.ScrollablePage {
    objectName: "tailMoves";
    title: i18nc("Heading for the Moves page", "Moves");
    actions {
        main: Kirigami.Action {
            text: i18nc("Button for returning the tail to its home position, on the Moves page", "Home Position");
            icon.name: "dialog-cancel";
            onTriggered: {
                BTConnectionManager.sendMessage("TAILHM", []);
            }
        }
    }
    BaseMovesComponent {
        infoText: i18nc("Description for the list of moves, on the Moves page", "The list below shows all the moves available to your gear. Tap any of them to send them off to any of your connected devices!");
        onCommandActivated: {
            CommandQueue.clear("");
            CommandQueue.pushCommand(command, destinations);
        }
        categoriesModel: [
            {
                name: i18nc("Description for the category for the Relaxed Moveset, on the Moves page", "Calm and Relaxed"),
                category: "relaxed",
                color: "#1cdc9a",
            },
            {
                name: i18nc("Description for the category for the Excited Moveset, on the Moves page", "Fast and Excited"),
                category: "excited",
                color: "#c9ce3b",
            },
            {
                name: i18nc("Description for the category for the Tense Moveset, on the Moves page", "Frustrated and Tense"),
                category: "tense",
                color: "#f67400",
            }
        ]
    }
}
