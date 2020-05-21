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
import QtQuick.Layouts 1.11
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
        buttonCancel.visible = !!okHandler;
        open();
    }

    header: Kirigami.Heading {
        id: textHeader;
    }

    footer: RowLayout {
        Layout.fillWidth: true

        QQC2.Button {
            text: qsTr("Ok");
            highlighted: true;
            Layout.fillWidth: true
            Layout.preferredWidth: control.width

            onClicked: {
                if (okHandler) {
                    var handler = okHandler;
                    okHandler = null;
                    close();
                    handler();
                } else {
                    close();
                }
            }
        }

        QQC2.Button {
            id: buttonCancel
            text: qsTr("Cancel");
            Layout.fillWidth: true
            Layout.preferredWidth: control.width

            onClicked: {
                close();
            }
        }
    }

    QQC2.Label {
        id: label;
        Layout.fillWidth: true;
        wrapMode: Text.Wrap;
    }
}
