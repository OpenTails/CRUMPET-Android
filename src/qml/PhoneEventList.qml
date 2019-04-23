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

import QtQuick 2.7
import QtQuick.Controls 2.4 as QQC2
import org.kde.kirigami 2.6 as Kirigami
import QtQuick.Layouts 1.11

Kirigami.ScrollablePage {
    id: root;

    objectName: "phoneEventList";
    title: qsTr("Phone Events");

    Component {
        id: phoneEventListDelegate;

        Kirigami.SwipeListItem {
            id: listItem;

            QQC2.Label {
                text: modelData["displayName"]
            }

            onClicked: {
                console.debug("Append this list, yo! ...maybe confirm, because it's a bit lots");
            }

            actions: [
                Kirigami.Action {
                    text: qsTr("Edit Phone Event Commands");
                    icon.name: ":/org/kde/kirigami/icons/document-edit.svg";

                    onTriggered: {
                        pageStack.push(editorPage, { phoneEvent: modelData });
                    }
                }
            ]
        }
    }

    Component {
        id: editorPage;
        PhoneEventEditor { }
    }

    ListView {
        model: AppSettings.phoneEventList;
        delegate: phoneEventListDelegate;
        header: InfoCard {
            text: qsTr("Set phone events here.");
        }
    }
}
