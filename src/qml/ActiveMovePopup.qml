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

import QtQuick 2.11
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.6 as Kirigami

// We should use Popup here instead of Kirigami.OverlaySheet
// because OverlaySheet does not support modal mode
QQC2.Popup {
    property bool blockOnMovingTail: false;
    property alias commandName: labelCommandName.text;

    visible: blockOnMovingTail && CommandQueue.currentCommandRemainingMSeconds > 0
    modal: true;
    closePolicy: Popup.NoAutoClose;

    ColumnLayout {
        implicitWidth: root.width - Kirigami.Units.largeSpacing * 4;

        Kirigami.Heading {
            text: qsTr("Tail Active...");
        }

        QQC2.Label {
            id: labelCommandName;
            wrapMode: Text.Wrap;
            Layout.fillWidth: true;
        }

        QQC2.ProgressBar {
            value: CommandQueue.currentCommandRemainingMSeconds;
            to: CommandQueue.currentCommandTotalDuration;

            Layout.fillWidth: true;
        }
    }
}
