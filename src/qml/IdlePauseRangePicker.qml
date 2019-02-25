/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Controls 2.4

import org.kde.kirigami 2.4 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

Column {
    Label {
        text: qsTr("Range of pause between random moves in seconds");

        anchors {
            left: parent.left;
            right: parent.right;
        }
    }

    RangeSlider {
        id: pauseRangeSlider;

        from: 15;
        to: 600;
        stepSize: 1.0;

        anchors {
            left: parent.left;
            right: parent.right;
            leftMargin: Kirigami.Units.largeSpacing;
        }

        first.onValueChanged: {
            AppSettings.idleMinPause = first.value;
        }

        second.onValueChanged: {
            if (second.value < 20) {
                second.value = 20;
            }

            AppSettings.idleMaxPause = second.value;
        }

        Component.onCompleted: {
            pauseRangeSlider.setValues(AppSettings.idleMinPause, AppSettings.idleMaxPause);
        }
    }

    Item {
        height: childrenRect.height;

        anchors {
            left: parent.left;
            right: parent.right;
            leftMargin: Kirigami.Units.largeSpacing;
        }

        Label {
            text: Math.floor(pauseRangeSlider.first.value);

            anchors {
                left: parent.left;
                right: parent.horizontalCentre;
            }
        }

        Label {
            text: Math.floor(pauseRangeSlider.second.value);

            anchors {
                left: parent.horizontalCentre;
                right: parent.right;
            }
        }
    }
}
