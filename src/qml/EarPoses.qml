/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.ScrollablePage {
    objectName: "earPoses";
    title: i18nc("Title for the page for selecting a pose for the EarGear", "Ear Poses");
    actions: [
        Kirigami.Action {
            text: i18nc("Button for returning the EarGear to the home position, on the page for selecting a pose for the EarGear", "Home Position");
            icon.name: "go-home";
            onTriggered: {
                Digitail.BTConnectionManager.sendMessage("TAILHM", []);
            }
        }
    ]
    BaseMovesComponent {
        infoText: i18nc("Description for the page for selecting a pose for the EarGear", "The list below shows all the poses available to your gear. Tap any of them to send them off to any of your connected devices! If you have more than one connected, the little coloured dots show which you can send that pose to.");
        infoFooter: RowLayout {
            QQC2.Button {
                text: i18nc("Label for the button for setting EarGear moves to run more slowly", "Be Calm")
                Layout.fillWidth: true
                Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                onClicked: {
                    Digitail.BTConnectionManager.sendMessage("SPEED SLOW", []);
                }
            }
            QQC2.Button {
                text: i18nc("Label for the button for setting EarGear moves to run faster", "Be Excited")
                Layout.fillWidth: true
                Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                onClicked: {
                    Digitail.BTConnectionManager.sendMessage("SPEED FAST", []);
                }
            }
        }
        onCommandActivated: {
            Digitail.CommandQueue.clear("");
            Digitail.CommandQueue.pushCommand(command, destinations);
        }
        categoriesModel: [
            {
                name: i18nc("Heading for the list of poses, on the page for selecting a pose for the EarGear", "Poses"),
                category: "eargearposes",
                color: "#93cee9",
            }
        ]
    }
}
