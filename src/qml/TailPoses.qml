/*
 *   Copyright 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ScrollablePage {
    title: qsTr("Tail Poses");
    actions {
        main: Kirigami.Action {
            text: "Sync";
            icon.name: "folder-sync";
            onTriggered: showPassiveNotification("Action clicked");
        }
    }
    ListView {
        id: poseList;
        currentIndex: 2;
        model: 10;
        implicitWidth: Kirigami.Units.gridUnit * 30
        delegate: Kirigami.BasicListItem {
            label: "Tail Pose " + modelData;
            checkable: true;
            checked: poseList.currentIndex == index;
            separatorVisible: false;
        }
    }
}
