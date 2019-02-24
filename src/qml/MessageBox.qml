/*
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

import QtQuick 2.11
import QtQuick.Controls 2.4 as QQC2
import org.kde.kirigami 2.6 as Kirigami

Kirigami.OverlaySheet {
    id: control;

    property alias headerText: textHeader.text;
    property alias text: label.text;
    property var okHandler: null;

    function showMessageBox(header, text, okHandler) {
        control.headerText = header;
        control.text = text;
        control.okHandler = okHandler;

        open();
    }

    header: Kirigami.Heading {
        id: textHeader;
    }

    Column {
        width: root.width - Kirigami.Units.largeSpacing * 4;
        spacing: Kirigami.Units.smallSpacing;

        QQC2.Label {
            id: label;
            width: parent.width;
            wrapMode: Text.Wrap;
        }

        QQC2.Button {
            text: qsTr("Ok");
            highlighted: true;
            width: parent.width;

            onClicked: {
                if (okHandler) {
                    handler = okHandler;
                    okHandler = null;
                    close();
                    handler();
                } else {
                    close();
                }
            }
        }
    }
}
