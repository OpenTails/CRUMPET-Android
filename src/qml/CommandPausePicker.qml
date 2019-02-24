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

import QtQuick 2.7
import QtQuick.Controls 2.4 as QQC2
import org.kde.kirigami 2.6 as Kirigami

Kirigami.OverlaySheet {
    id: control;

    property int insertAt;

    signal durationPicked(int duration);
    signal insertCommand(int insertAt, string command);

    function pickDuration() {
        durationSlider.value = 10;
        open();
    }

    onDurationPicked: {
        control.insertCommand(insertAt, "pause:" + duration);
    }

    header: Kirigami.Heading {
        text: qsTr("Pick the duration of your pause");
    }

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
            text: qsTr("%1 seconds").arg(durationSlider.value);
        }

        QQC2.Button {
            text: qsTr("Add this pause");

            onClicked: {
                control.durationPicked(durationSlider.value);
            }
        }
    }
}
