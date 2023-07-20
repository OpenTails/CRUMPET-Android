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
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    objectName: "tailLights";
    title: i18nc("Header for the page for the Glow Tips", "Glow Tips");
    actions {
        main: Kirigami.Action {
            text: i18nc("Button for stopping the light patterns, on the page for the Glow Tips", "Stop Lights");
            icon.name: "flashlight-off";
            onTriggered: {
                Digitail.BTConnectionManager.sendMessage("LEDOFF", []);
            }
        }
    }
    BaseMovesComponent {
        width: parent.width;
        infoText: i18nc("Description for the list of light patterns, on the page for the Glow Tips", "The list below shows all the light patterns available to your gear. Tap any of them to send them off to any of your connected devices!");
        onCommandActivated: {
            Digitail.CommandQueue.clear("");
            Digitail.CommandQueue.pushCommand(command, destinations);
        }
        categoriesModel: [
            {
                name: i18nc("List element for the light patterns, on the page for the Glow Tips", "LED Patterns"),
                category: "lights",
                color: "#93cee9",
            }
        ]
    }
}
