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
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.OverlaySheet {
    id: root;

    function pickCommand() {
        open();
    }
    signal commandPicked(string command, variant destinations);
    showCloseButton: true
    title: i18nc("Heading for the overlay for picking a command, for configuring a move list", "Pick a command");
    BaseMovesComponent {
        infoText: i18nc("Description for the overlay for picking a command, for configuring a move list", "The list below shows all the moves and light patterns available to your gear. Tap any of them to pick that command.");
        width: root.width;
        ignoreAvailability: true;
        onCommandActivated: function(command, destinations) {
            pickACommand.commandPicked(command, destinations);
        }
        categoriesModel: connectedWithLightsModel.count > 0
            ? [
                {
                    name: i18nc("Description for the category for the Ear Gear poses, on the overlay for picking a command, for configuring a move list", "Poses"),
                    category: "eargearposes",
                    color: "#93cee9",
                },
                {
                    name: i18nc("Description for the category for the Relaxed Moveset, on the overlay for picking a command, for configuring a move list", "Calm and Relaxed"),
                    category: "relaxed",
                    color: "#1cdc9a",
                },
                {
                    name: i18nc("Description for the category for the Excited Moveset, on the overlay for picking a command, for configuring a move list", "Fast and Excited"),
                    category: "excited",
                    color: "#c9ce3b",
                },
                {
                    name: i18nc("Description for the category for the Tense Moveset, on the overlay for picking a command, for configuring a move list", "Frustrated and Tense"),
                    category: "tense",
                    color: "#f67400",
                },
                {
                    name: i18nc("Description for the category for the LED Patterns, on the overlay for picking a command, for configuring a move list", "LED Patterns"),
                    category: "lights",
                    color: "#93cee9",
                }
            ]
            : [
                {
                    name: i18nc("Description for the category for the Ear Gear poses, on the overlay for picking a command, for configuring a move list", "Poses"),
                    category: "eargearposes",
                    color: "#93cee9",
                },
                {
                    name: i18nc("Description for the category for the Relaxed Moveset, on the overlay for picking a command, for configuring a move list", "Calm and Relaxed"),
                    category: "relaxed",
                    color: "#1cdc9a",
                },
                {
                    name: i18nc("Description for the category for the Excited Moveset, on the overlay for picking a command, for configuring a move list", "Fast and Excited"),
                    category: "excited",
                    color: "#c9ce3b",
                },
                {
                    name: i18nc("Description for the category for the Tense Moveset, on the overlay for picking a command, for configuring a move list", "Frustrated and Tense"),
                    category: "tense",
                    color: "#f67400",
                }
            ]
        Digitail.FilterProxyModel {
            id: connectedWithLightsModel;
            sourceModel: connectedDevicesModel;
            filterRole: Digitail.DeviceModelTypes.HasLights;
            filterBoolean: true;
        }
        Digitail.FilterProxyModel {
            id: connectedDevicesModel
            sourceModel: Digitail.DeviceModel;
            filterRole: Digitail.DeviceModelTypes.IsConnected;
            filterBoolean: true;
        }
    }
}
