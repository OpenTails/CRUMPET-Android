/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.4 as QQC2
import org.kde.kirigami 2.13 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

BaseCommandListEditor {
    property string moveListName;

    objectName: "moveListEditor";
    title: moveListName;
    model: AppSettings.moveList
    infoCardFooter: QQC2.Button {
        text: qsTr("Run Move List")
        Layout.fillWidth: true
        onClicked: {
            showMessageBox(qsTr("Run this list?"),
                qsTr("Do you want to run this list?"),
                function() {
                    CommandQueue.pushCommands(AppSettings.moveList);
                });
        }
    }

    onMoveListNameChanged: {
        AppSettings.setActiveMoveList(moveListName);
    }

    onInsertCommand: {
        AppSettings.addMoveListEntry(insertAt, command, destinations);
    }
    onRemoveCommand: {
        showMessageBox(qsTr("Remove Move List Entry?"),
                        qsTr("Are you sure that you want to remove this entry from your move list?"),
                        function () {
                            AppSettings.removeMoveListEntry(index);
                        });
    }
}
