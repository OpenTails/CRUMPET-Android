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
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    id: component;
    title: i18nc("Heading for the page for configuring command Sets", "Gear Command Sets")
    property string deviceID;

    ListView {
        id: crumpetList;
        model: Object.keys(Digitail.AppSettings.commandFiles);
        Digitail.FilterProxyModel {
            id: deviceFilterProxy;
            sourceModel: Digitail.DeviceModel;
            filterRole: 258; // the deviceID role
            filterString: component.deviceID;
            property var enabledFiles: [];
            property string deviceName: "unknown";
            onDataChanged: {
                enabledFiles = data(index(0, 0), 267);
                deviceName = data(index(0, 0), 257);
            }
        }
        header: InfoCard {
            text: i18nc("Description for the page for configuring Command Sets, for a specific gear", "This is all the command sets you have available to enable for %1. You can add new ones, edit them, and remove them. You cannot edit the built-in lists, but you can duplicate them and then edit those.", deviceFilterProxy.deviceName);
        }
        delegate: Kirigami.SwipeListItem {
            id: listItem;
            property var commandFile: Digitail.AppSettings.commandFiles[modelData];
            property bool isEnabled: deviceFilterProxy.enabledFiles ? deviceFilterProxy.enabledFiles.includes(modelData) : false;
            Layout.fillWidth: true;
            RowLayout {
                Layout.fillWidth: true;
                Kirigami.Icon {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium - Kirigami.Units.smallSpacing * 2;
                    Layout.minimumWidth: Kirigami.Units.iconSizes.small;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.small;
                    source: listItem.isEnabled ? ":/icons/breeze-internal/emblems/16/checkbox-checked" : ":/icons/breeze-internal/emblems/16/checkbox-unchecked";
                }
                Kirigami.Icon {
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium - Kirigami.Units.smallSpacing * 2;
                    Layout.minimumWidth: Kirigami.Units.iconSizes.medium;
                    Layout.maximumWidth: Kirigami.Units.iconSizes.medium;
                    source: ":/images/crumpet-head.svg";
                }
                QQC2.Label {
                    Layout.fillWidth: true;
                    wrapMode: Text.Wrap;
                    text: commandFile.title
                }
            }
            onClicked: {
                Digitail.BTConnectionManager.setDeviceCommandsFileEnabled(component.deviceID, modelData, !listItem.isEnabled);
            }
            actions: [
                Kirigami.Action {
                    visible: commandFile.isEditable;
                    text: i18nc("Button for deleting a Command Set, on the page for configuring Command Sets", "Delete");
                    icon.name: "list-remove";
                    onTriggered: {
                        showMessageBox(i18nc("Header for the confirmation prompt for removing a command set, on the Gear Command Sets page", "Remove Command Set?"),
                            i18nc("Message for the confirmation prompt for removing a command set, on the Gear Command Sets page", "Are you sure that you want to remove this command set? Note this cannot be undone."),
                            function () {
                                Digitail.AppSettings.removeCommandFile(modelData);
                            });
                    }
                },
                Kirigami.Action {
                    text: i18nc("Button for duplicating a Command Set, on the page for configuring Command Sets", "Duplicate");
                    icon.name: "edit-duplicate";
                    onTriggered: {
                        var newFileName = "internal-crumpet-" + crumpetList.count;
                        Digitail.AppSettings.addCommandFile(newFileName, commandFile.contents);
                    }
                },
                Kirigami.Action {
                    visible: commandFile.isEditable;
                    text: i18nc("Button for editing a Command Set, on the page for configuring Command Sets", "Edit Commands");
                    icon.name: "document-edit";
                    onTriggered: { pageStack.push( crumpetEditor, { filename: modelData } ); }
                }
            ]
        }
        Component {
            id: crumpetEditor;
            Kirigami.ScrollablePage {
                id: editorPage;
                property string filename;
                objectName: "crumpetEditor";
                title: i18nc("Header for the overlay for editing a Command Set, on the page for configuring Command Sets", "Edit Commands")

                Component.onCompleted: {
                    contentEditor.text = Digitail.AppSettings.commandFiles[editorPage.filename].contents;
                }

                actions {
                    main: Kirigami.Action {
                        text: i18nc("Button for saving a Command Set, on the overlay for editing a Command Set, on the page for configuring Command Sets", "Save");
                        icon.name: "file-save";
                        onTriggered: {
                            Digitail.AppSettings.setCommandFileContents(editorPage.filename, contentEditor.text);
                            if (pageStack.currentItem.objectName === editorPage.objectName) {
                                pageStack.pop();
                            }
                        }
                    }
                }

                TextEdit {
                    id: contentEditor;
                    Layout.fillWidth: true;
                    text: i18nc("Placeholder text for the overlay for editing a Command Set, on the page for configuring Command Sets", "(this is where the content of the crumpet file goes,\nbecause we'll just have that here for now...)");
                    textFormat: TextEdit.PlainText;
                    wrapMode: TextEdit.Wrap
                    focus: true;
                    selectByMouse: true;
                    persistentSelection: true;

                    function ensureVisible(rectToMakeVisible)
                    {
                        if (editorPage.flickable.contentX >= rectToMakeVisible.x) {
                            editorPage.flickable.contentX = rectToMakeVisible.x;
                        } else if (editorPage.flickable.contentX + editorPage.flickable.width <= rectToMakeVisible.x + rectToMakeVisible.width) {
                            editorPage.flickable.contentX = rectToMakeVisible.x + rectToMakeVisible.width - editorPage.flickable.width;
                        }
                        if (editorPage.flickable.contentY >= rectToMakeVisible.y) {
                            editorPage.flickable.contentY = rectToMakeVisible.y;
                        } else if (editorPage.flickable.contentY + editorPage.flickable.height <= rectToMakeVisible.y + rectToMakeVisible.height) {
                            editorPage.flickable.contentY = rectToMakeVisible.y + rectToMakeVisible.height - editorPage.flickable.height;
                        }
                    }
                    onCursorRectangleChanged: {
                        ensureVisible(cursorRectangle);
                    }
                }
            }
        }
    }
}
