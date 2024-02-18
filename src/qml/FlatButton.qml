/*
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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
import QtQuick.Layouts
import QtQuick.Controls.Material

Button {
    id: control

    property alias textColor: controlText.color
    property alias textAlignment: controlText.horizontalAlignment

    focusPolicy: Qt.NoFocus

    leftPadding: 6
    rightPadding: 6

    Layout.fillWidth: true
    Layout.preferredWidth: 1

    contentItem: Text {
        id: controlText

        text: control.text
        color: Material.foreground
        opacity: enabled ? 1.0 : 0.3
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.capitalization: Font.AllUppercase
        font.weight: Font.Medium
    }

    background: Rectangle {
        color: control.pressed ? controlText.color : "transparent"
        opacity: control.pressed ? 0.12 : 1.0
        radius: 2
        implicitHeight: 48
        Layout.minimumWidth: 88
    }
}
