/*
 *   Copyright 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
import org.kde.kirigami 2.5 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    objectName: "idleMode";
    title: qsTr("Idle Mode Categories");

    Component {
        id: idleCategoryDelegate;
        Kirigami.BasicListItem {
            icon: Digitail.AppSettings.idleCategories.includes(model.category) ? ":/org/kde/kirigami/icons/checkbox-checked.svg" : ":/org/kde/kirigami/icons/checkbox-unchecked.svg";
            label: model.name;
            onClicked: {
                var categories = Digitail.AppSettings.idleCategories;
                if(Digitail.AppSettings.idleCategories.includes(model.category)) {
                    var idx = categories.indexOf(model.category);
                    categories.splice(idx, 1);
                }
                else {
                    categories.push(model.category);
                }
                Digitail.AppSettings.idleCategories = categories;
            }
        }
    }
    ListModel {
        id: availableCategories;
        ListElement {
            name: qsTr("Calm and Relaxed");
            category: "relaxed";
            color: "#1cdc9a";
        }
        ListElement {
            name: qsTr("Fast and Excited");
            category: "excited";
            color: "#c9ce3b";
        }
        ListElement {
            name: qsTr("Frustrated and Tense");
            category: "tense";
            color: "#f67400";
        }
        ListElement {
            name: qsTr("LED Patterns");
            category: "lights";
            color: "#93cee9";
        }
    }

    ListView {
        model: availableCategories;
        delegate: idleCategoryDelegate;
    }
}

