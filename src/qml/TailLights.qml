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

Kirigami.ScrollablePage {
    objectName: "tailLights";
    title: i18nc("Header for the page for the Glow Tips", "Glow Tips");
    actions: [
        Kirigami.Action {
            text: i18nc("Button for stopping the light patterns, on the page for the Glow Tips", "Stop Lights");
            icon.name: "flashlight-off";
            displayHint: Kirigami.DisplayHint.KeepVisible;
            onTriggered: {
                Digitail.BTConnectionManager.sendMessage("LEDOFF", []);
            }
        }
    ]
    BaseMovesComponent {
        width: parent.width;
        infoText: i18nc("Description for the list of light patterns, on the page for the Glow Tips", "The list below shows all the light patterns available to your gear. Tap any of them to send them off to any of your connected devices! If you have more than one connected, the little coloured dots show which you can send that light pattern to.");
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
