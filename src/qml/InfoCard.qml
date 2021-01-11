/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami

ColumnLayout {
    id: root;
    property alias text: infoCardText.text;
    property alias footer: card.footer;
    width: parent.width;
    height: card.height + Kirigami.Units.largeSpacing * 3;
    Kirigami.Card {
        id: card
        Layout.fillWidth: true;
        Layout.margins: Kirigami.Units.largeSpacing;
        contentItem: QQC2.Label {
            id: infoCardText;
            padding: Kirigami.Units.smallSpacing;
            wrapMode: Text.Wrap;
        }
    }
}
