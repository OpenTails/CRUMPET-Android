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
import org.kde.kirigami 2.13 as Kirigami

Kirigami.OverlaySheet {
    id: control;

    property alias headerText: textHeader.text;
    property alias description: textDescription.text;
    property alias placeholderText: enteredName.placeholderText;
    property alias buttonOkText: buttonOk.text;

    signal namePicked(string name);

    function pickName() {
        enteredName.text = "";
        open();
    }

    header: Kirigami.Heading {
        id: textHeader;
        text: i18nc("Text header for picking a name", "Pick a name");
        wrapMode: Text.Wrap;
    }

    footer: QQC2.Button {
        id: buttonOk;
        width: parent.width;

        // The check below is done to ensure we do not have to wait for the text to be accepted
        // before enabling the button. The text will still be entered, but both need to be checked
        // to ensure the expected behaviour (as the actual text does not get entered until the
        // text box loses focus or the input is accepted).
        enabled: (enteredName.preeditText + enteredName.text).length;

        onClicked: {
            control.namePicked(enteredName.text);
        }
    }

    Column {
        width: root.width - Kirigami.Units.largeSpacing * 4;
        spacing: Kirigami.Units.smallSpacing;

        QQC2.Label {
            id: textDescription;
            width: parent.width;
            wrapMode: Text.Wrap;
        }

        QQC2.TextField {
            id: enteredName;
            width: parent.width;
        }
    }
}
