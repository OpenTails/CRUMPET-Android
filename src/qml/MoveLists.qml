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

Kirigami.ScrollablePage {
    id: root;
    objectName: "moveLists";
    title: qsTr("Move Lists");

    actions {
        main: Kirigami.Action {
            text: qsTr("Add New Move List");
            icon.name: ":/org/kde/kirigami/icons/list-add.svg";
            onTriggered: {
                namePicker.pickName();
            }
        }
    }
    Component {
        id: moveListDelegate;
        Kirigami.SwipeListItem {
            id: listItem;
            property string title: modelData
            QQC2.Label {
                text: modelData
            }
            onClicked: {
                AppSettings.setActiveMoveList(modelData);
                showMessageBox(qsTr("Run this list?"),
                               qsTr("Do you want to run the list %1?").arg(modelData),
                               function() {
                                   CommandQueue.pushCommands(AppSettings.moveList);
                               });
            }
            actions: [
                Kirigami.Action {
                    text: qsTr("Edit Move List");
                    icon.name: ":/org/kde/kirigami/icons/document-edit.svg";
                    onTriggered: {
                        pageStack.push(editorPage, { moveListName: modelData });
                    }
                },
                Kirigami.Action { },
                Kirigami.Action {
                    text: qsTr("Delete this Move List");
                    icon.name: ":/org/kde/kirigami/icons/list-remove.svg";

                    onTriggered: {
                        showMessageBox(qsTr("Remove the Move List"),
                                       qsTr("Are you sure that you want to remove the Move List '%1'?").arg(modelData),
                                       function () {
                                           AppSettings.removeMoveList(modelData);
                                       });
                    }
                }
            ]
        }
    }
    Component {
        id: editorPage;
        MoveListEditor { }
    }
    ListView {
        delegate: moveListDelegate;
        model: AppSettings.moveLists;
        header: InfoCard {
            title: qsTr("Playlists for your tail");
            text: qsTr("Create lists of moves, glows and pauses to send to your tail. To send a list you have created, tap it and pick OK. To perform actions on the ones you've created, like editing or even removing them entirely, swipe left on the item.");
        }
    }

    NamePicker {
        id: namePicker;

        description: qsTr("Enter a name to use for your new move list and click Create");
        placeholderText: qsTr("Enter your move list name here");
        buttonOkText: qsTr("Create");

        onNamePicked: {
            AppSettings.addMoveList(name);
            namePicker.close();
        }
    }
}
