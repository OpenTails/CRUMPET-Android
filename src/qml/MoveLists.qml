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
import org.kde.kirigami as Kirigami
import org.thetailcompany.digitail as Digitail

Kirigami.ScrollablePage {
    id: root;
    objectName: "moveLists";
    title: i18nc("Heading for the page for viewing Move Lists", "Move Lists");

    actions: [
        Kirigami.Action {
            text: i18nc("Label for the button for adding a new Move List, on the page for viewing Move Lists", "Add New Move List");
            icon.name: "list-add";
            displayHint: Kirigami.DisplayHint.KeepVisible;
            onTriggered: {
                namePicker.pickName();
            }
        }
    ]
    Component {
        id: moveListDelegate;
        Kirigami.SwipeListItem {
            id: listItem;
            property string title: modelData
            QQC2.Label {
                text: modelData
            }
            onClicked: {
                Digitail.AppSettings.setActiveMoveList(modelData);
                showMessageBox(i18nc("Heading for the prompt for confirming the action of running a list, on the page for viewing Move Lists", "Run this list?"),
                               i18nc("Message for the prompt for confirming the action of running a list, on the page for viewing Move Lists", "Do you want to run the list %1?", modelData),
                               function() {
                                   Digitail.CommandQueue.pushCommands(AppSettings.moveList, []);
                               });
            }
            actions: [
                Kirigami.Action {
                    text: i18nc("Label for the button for editing a move list, on the page for viewing Move Lists", "Edit Move List");
                    icon.name: "document-edit";
                    displayHint: Kirigami.DisplayHint.KeepVisible;
                    onTriggered: {
                        pageStack.push(editorPage, { moveListName: modelData });
                    }
                },
                Kirigami.Action { separator: true; enabled: false; },
                Kirigami.Action {
                    text: i18nc("Label for the button for deleting a move list, on the page for viewing Move Lists", "Delete this Move List");
                    icon.name: "list-remove";
                    displayHint: Kirigami.DisplayHint.KeepVisible;
                    onTriggered: {
                        showMessageBox(i18nc("Heading for the prompt for confirming the action of deleting a list, on the page for viewing Move Lists", "Remove the Move List"),
                                       i18nc("Message for the prompt for confirming the action of deleting a list, on the page for viewing Move Lists", "Are you sure that you want to remove the Move List '%1'?", modelData),
                                       function () {
                                           Digitail.AppSettings.removeMoveList(modelData);
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
        model: Digitail.AppSettings.moveLists;
        header: InfoCard {
            text: i18nc("Description for the page for viewing Move Lists", "Create lists of moves, glows and pauses to send to your gear. To send a list you have created, tap it and pick OK. To perform actions on the ones you've created, like editing or even removing them entirely, swipe left on the item.");
        }
    }

    NamePicker {
        id: namePicker;

        description: i18nc("Description for the prompt for giving a name to a move list, on the page for viewing Move Lists", "Enter a name to use for your new move list and click Create");
        placeholderText: i18nc("Placeholder text for the textfield for the prompt for giving a name to a move list, on the page for viewing Move Lists", "Enter your move list name here");
        buttonOkText: i18nc("Confirmating button for the prompt for giving a name to a move list, on the page for viewing Move Lists", "Create");

        onNamePicked: function(name) {
            if (Digitail.AppSettings.moveLists.includes(name)) {
                showMessageBox(i18nc("Header for the warning that a name is already in use, for the prompt for giving a name to a move list, on the page for viewing Move Lists", "Name Already Used"),
                               i18nc("Description for the warning that a name is already in use, for the prompt for giving a name to a move list, on the page for viewing Move Lists","You already have a move list with the name %1. Please pick another name, as that won't work.", name));
            } else {
                Digitail.AppSettings.addMoveList(name);
                namePicker.close();
            }
        }
    }
}
