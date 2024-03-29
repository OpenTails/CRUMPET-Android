/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control;

    property int insertAt;

    signal durationPicked(int duration);
    signal insertCommand(int insertAt, string command);

    function pickDuration() {
        durationSlider.value = 10;
        open();
    }

    onDurationPicked: function(duration) {
        control.insertCommand(insertAt, "pause:" + duration);
    }

    showCloseButton: true
    title: i18nc("Heading for an overlay for picking the duration of a pause in the command list", "Pick the duration of your pause");

    Column {
        QQC2.Slider {
            id: durationSlider;

            width: control.width - Kirigami.Units.largeSpacing * 4;
            from: 1;
            to: 300;
            stepSize: 1;
            snapMode: QQC2.Slider.SnapAlways;
        }

        QQC2.Label {
            width: control.width - Kirigami.Units.largeSpacing * 4;
            text: i18nc("A label which describes the slider for the duration of a pause, in the overlay for picking the duration of a pause, in a command list", "%1 seconds", durationSlider.value);
        }

        QQC2.Button {
            text: i18nc("Button for an overlay for picking the duration of a pause in the command list", "Add this pause");

            width: control.width - Kirigami.Units.largeSpacing * 4;
            onClicked: {
                control.durationPicked(durationSlider.value);
            }
        }
    }
}
