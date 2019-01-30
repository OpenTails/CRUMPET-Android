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
import QtQuick.Controls 2.4
import org.kde.kirigami 2.5 as Kirigami
import org.thetailcompany.digitail 1.0 as Digitail

Kirigami.ScrollablePage {
    objectName: "idleMode";
    title: qsTr("Idle Mode Categories");

    Component {
        id: idleCategoryDelegate;
        Kirigami.BasicListItem {
            icon: AppSettings.idleCategories.indexOf(model.category) >= 0 ? ":/org/kde/kirigami/icons/checkbox-checked.svg" : ":/org/kde/kirigami/icons/checkbox-unchecked.svg";
            label: model.name;
            onClicked: {
                var idx = AppSettings.idleCategories.indexOf(model.category);
                if(idx >= 0) {
                    AppSettings.removeIdleCategory(model.category);
                }
                else {
                    AppSettings.addIdleCategory(model.category);
                }
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
        header: Kirigami.AbstractCard {
            anchors { left: parent.left; right: parent.right; }
            contentItem: Column {
                Label {
                    text: qsTr("Range of pause between random moves in seconds");
                    anchors { left: parent.left; right: parent.right; }
                }
                RangeSlider {
                    id: pauseRangeSlider;
                    anchors { left: parent.left; right: parent.right; leftMargin: Kirigami.Units.largeSpacing; }
                    from: 0;
                    to: 120;
                    stepSize: 1.0;
                    first.onValueChanged: { AppSettings.idleMinPause = first.value; }
                    second.onValueChanged: { AppSettings.idleMaxPause = second.value; }
                    Component.onCompleted: {
                        pauseRangeSlider.setValues(AppSettings.idleMinPause, AppSettings.idleMaxPause);
                    }
                }
                Item {
                    anchors { left: parent.left; right: parent.right; leftMargin: Kirigami.Units.largeSpacing; }
                    height: childrenRect.height;
                    Label {
                        text: Math.floor(pauseRangeSlider.first.value);
                        anchors {
                            left: parent.left;
                            right: parent.horizontalCentre;
                        }
                    }
                    Label {
                        text: Math.floor(pauseRangeSlider.second.value);
                        verticalAlignment: Text.AlignRight;
                        anchors {
                            left: parent.horizontalCentre;
                            right: parent.right;
                        }
                    }
                }
            }
        }
    }
}

