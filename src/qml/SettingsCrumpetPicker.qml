/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Controls 2.11 as QQC2
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami

Kirigami.ScrollablePage {
    id: component;
    title: qsTr("Gear Command Sets")

    ListView {
        model: 5
        header: QQC2.Label {
            width: parent.width;
            padding: Kirigami.Units.smallSpacing;
            wrapMode: Text.Wrap;
            text: qsTr("This is all the command sets you have available. You can add new ones, edit them, and remove them. You cannot edit the built-in lists, but you can duplicate them and then edit those.");
        }
        delegate: Kirigami.SwipeListItem {
            id: listItem;
            Layout.fillWidth: true;
            RowLayout {
                Layout.fillWidth: true;
                Kirigami.Icon {
                    Layout.preferredHeight: listItem.height - Kirigami.Units.smallSpacing * 2;
                    Layout.minimumWidth: Kirigami.Units.iconSizes.small;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.small;
                    source: modelData % 2 === 0 ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                }
                Kirigami.Icon {
                    Layout.preferredHeight: listItem.height - Kirigami.Units.smallSpacing * 2;
                    Layout.minimumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.medium;
                    source: ":/images/crumpet-head.svg";
                }
                QQC2.Label {
                    Layout.fillWidth: true;
                    text: modelData
                }
            }
            actions: [
                Kirigami.Action {
                    text: qsTr("Delete");
                    icon.name: "list-remove";
                },
                Kirigami.Action {
                    text: qsTr("Duplicate");
                    icon.name: "edit-duplicate";
                },
                Kirigami.Action {
                    text: qsTr("Edit Commands");
                    icon.name: "document-edit";
                    onTriggered: { crumpetEditor.open(); }
                }
            ]
        }
        Kirigami.OverlaySheet {
            id: crumpetEditor;

            header: Kirigami.Heading {
                text: qsTr("Edit Commands")
            }

            footer: RowLayout {
                Layout.fillWidth: true

                QQC2.Button {
                    text: qsTr("Save");
                    highlighted: true;
                    Layout.fillWidth: true
                    onClicked: {
                        crumpetEditor.close();
                    }
                }

                QQC2.Button {
                    text: qsTr("Cancel");
                    Layout.fillWidth: true
                    onClicked: {
                        crumpetEditor.close();
                    }
                }
            }

            QQC2.TextArea {
                width: component.width - Kirigami.Units.largeSpacing * 4;
                Layout.fillHeight: true;
                text: "(this is where the content of the crumpet file goes,\nbecause we'll just have that here for now...)"
            }
        }
    }
}
