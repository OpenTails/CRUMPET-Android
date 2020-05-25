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
import org.thetailcompany.digitail 1.0

Kirigami.ScrollablePage {
    id: component;
    title: qsTr("Gear Command Sets")
    property string deviceID;

    ListView {
        id: crumpetList;
        model: Object.keys(AppSettings.commandFiles);
        FilterProxyModel {
            id: deviceFilterProxy;
            sourceModel: DeviceModel;
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
            text: qsTr("This is all the command sets you have available to enable for %1. You can add new ones, edit them, and remove them. You cannot edit the built-in lists, but you can duplicate them and then edit those.").arg(deviceFilterProxy.deviceName);
        }
        delegate: Kirigami.SwipeListItem {
            id: listItem;
            property var commandFile: AppSettings.commandFiles[modelData];
            property bool isEnabled: deviceFilterProxy.enabledFiles.includes(modelData);
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
                    text: commandFile.title
                }
            }
            onClicked: {
                BTConnectionManager.setDeviceCommandsFileEnabled(component.deviceID, modelData, !listItem.isEnabled);
            }
            actions: [
                Kirigami.Action {
                    visible: commandFile.isEditable;
                    text: qsTr("Delete");
                    icon.name: "list-remove";
                },
                Kirigami.Action {
                    text: qsTr("Duplicate");
                    icon.name: "edit-duplicate";
                    onTriggered: {
                        var newFileName = "internal-crumpet-" + crumpetList.count;
                        AppSettings.addCommandFile(newFileName, commandFile.contents);
                    }
                },
                Kirigami.Action {
                    visible: commandFile.isEditable;
                    text: qsTr("Edit Commands");
                    icon.name: "document-edit";
                    onTriggered: { crumpetEditor.openEditor(modelData); }
                }
            ]
        }
        Kirigami.OverlaySheet {
            id: crumpetEditor;
            property string _filename;
            function openEditor(filename) {
                _filename = filename;
                contentEditor.text = AppSettings.commandFiles[filename].contents;
                crumpetEditor.open();
            }

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
                        AppSettings.setCommandFileContents(crumpetEditor._filename, contentEditor.text);
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
                id: contentEditor;
                width: component.width - Kirigami.Units.largeSpacing * 4;
                Layout.fillHeight: true;
                text: "(this is where the content of the crumpet file goes,\nbecause we'll just have that here for now...)";
                selectByMouse: true;
            }
        }
    }
}
