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
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

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

        // The check below is done to ensure we do not have to wait for the text to be accepted
        // before enabling the button. The text will still be entered, but both need to be checked
        // to ensure the expected behaviour (as the actual text does not get entered until the
        // text box loses focus or the input is accepted).
        enabled: (enteredName.preeditText + enteredName.text).length;

        onClicked: {
            control.namePicked(enteredName.text);
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing;
        QQC2.Label {
            id: textDescription;
            Layout.fillWidth: true
            wrapMode: Text.Wrap;
        }
        QQC2.TextField {
            id: enteredName;
            Layout.fillWidth: true
        }
        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: Kirigami.Units.smallSpacing
        }
    }
}
