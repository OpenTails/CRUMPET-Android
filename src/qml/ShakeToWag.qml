/*
 *   Copyright 2020 Evgeni B<evgeni.biryuk.tail@gofree.club>
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
import QtQuick.Controls 2.11
import QtQuick.Layouts 1.11
import QtQuick.Extras 1.4
import QtSensors 5.12
import org.kde.kirigami 2.13 as Kirigami

Kirigami.ScrollablePage {
    id: component;
    objectName: "shakeToWag";
    title: qsTr("Shake To Wag");
    actions {
        main: Kirigami.Action {
            text: qsTr("Home Position");
            icon.name: "go-home";
            onTriggered: {
                BTConnectionManager.sendMessage("TAILHM", []);
            }
        }
    }

    property var gyroscope_x_values: [];
    property var gyroscope_y_values: [];
    property var gyroscope_z_values: [];
    property string jumpMove: "TAILER"
    property string shakeMove: "TAILS1"

    property bool isReadingMode: false;

    function recognizeMovement() {
        var minX = 0;
        var maxX = 0;
        var minY = 0;
        var maxY = 0;
        var minZ = 0;
        var maxZ = 0;

        var minAmpl = 0;
        var maxAmpl = 0;

        var axValues = [];
        var ayValues = [];
        var azValues = [];

        var count = gyroscope_x_values.length;
        for (var i = 0; i < count; i++) {
            if (gyroscope_x_values[i] <= minX) {
                minX = gyroscope_x_values[i];
            } else {
                maxX = gyroscope_x_values[i];
            }

            if (gyroscope_y_values[i] <= minY) {
                minY = gyroscope_y_values[i];
            } else {
                maxY = gyroscope_y_values[i];
            }

            if (gyroscope_z_values[i] <= minZ) {
                minZ = gyroscope_z_values[i];
            } else {
                maxZ = gyroscope_z_values[i];
            }
        }

        var amplitudeX = maxX - minX;
        var amplitudeY = maxY - minY;
        var amplitudeZ = maxZ - minZ;

        minAmpl = Math.min(amplitudeX, amplitudeY, amplitudeZ);
        maxAmpl = Math.max(amplitudeX, amplitudeY, amplitudeZ);

        //temporarily commented out
        //amplitudeGyroscopeData.text = "Amplitude gyroscope: x: " + Math.round(amplitudeX) + ", y: " + Math.round(amplitudeY) + ", z: " + Math.round(amplitudeZ);

        if (maxAmpl - minAmpl > 700) {
            return "Jump";
        } else {
            return "Shake";
        }
    }

    ScrollView {
        id: scrollView;
        ColumnLayout {
            width: component.width - Kirigami.Units.largeSpacing * 4
            RowLayout {
            Layout.fillWidth: true;
                Text {
                    Layout.fillWidth: true;
                    text: qsTr("Jump move:")
                }
                Button {
                    Layout.fillWidth: true;
                    text: component.jumpMove;
                    onClicked: {
                        pickACommand.pickWhat = "jump";
                        pickACommand.pickCommand();
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true;
                Text {
                    Layout.fillWidth: true;
                    text: qsTr("Shake move:")
                }
                Button {
                    Layout.fillWidth: true;
                    text: component.shakeMove;
                    onClicked: {
                        pickACommand.pickWhat = "shake";
                        pickACommand.pickCommand();
                    }
                }
            }
            Item { height: Kirigami.Units.largeSpacing; width: Kirigami.Units.gridUnit }

            ToggleButton {
                id: goButton;
                text: qsTr("GO");
                Layout.alignment: Qt.AlignHCenter
                onClicked: function() {
                    isReadingMode = goButton.checked;
                    goButton.text = qsTr(isReadingMode ? "STOP": "GO");

                    if (isReadingMode) {
                        movementStatus.text = qsTr("Make a move");
                        gyroscope_x_values = [];
                        gyroscope_y_values = [];
                        gyroscope_z_values = [];
                    } else {
                        movementStatus.text = qsTr("Analyzing...");
                        var movement = recognizeMovement();
                        if (movement == "Jump") {
                            BTConnectionManager.sendMessage(component.jumpMove, []);
                        } else if (movement == "Shake") {
                            BTConnectionManager.sendMessage(component.shakeMove, []);
                        }

                        movementStatus.text = "Your movement" + '\n' + "was recognized" + '\n' + "like a " + movement;
                    }
                }
            }


    //temporarily commented out
    //        Text {
    //            id: gyroscopeData
    //            text: "Gyroscope: x: 0, y: 0, z: 0";
    //        }

    //        Text {
    //            id: amplitudeGyroscopeData
    //            text: "Amplitude Gyroscope: x: 0, y: 0, z: 0";
    //        }

    //        Text {
    //            id: vectorChangesData
    //            text: "Vector Changes: x: 0, y: 0, z: 0";
    //        }

            Text {
                id: movementStatus;
                font.pointSize: 36;
                Layout.alignment: Qt.AlignHCenter
            }
            Item { height: Kirigami.Units.largeSpacing; width: Kirigami.Units.gridUnit }

        }

        Gyroscope {
            id: gyroscope
            dataRate: 100
            active: true
            onReadingChanged: {
                if (isReadingMode) {
                    var x = gyroscope.reading.x;
                    var y = gyroscope.reading.y;
                    var z = gyroscope.reading.z;

                    gyroscope_x_values.push(x);
                    gyroscope_y_values.push(y);
                    gyroscope_z_values.push(z);

                    console.log("x: ", x);
                    console.log("y: ", y);
                    console.log("z: ", z);

                    //temporarily commented out
                    //gyroscopeData.text = "Gyroscope: x: " + Math.round(x) + ", y: " + Math.round(y) + ", z: " + Math.round(z);
                }
            }
        }
    }

    PickACommandSheet {
        id: pickACommand;

        property string pickWhat;

        onCommandPicked: {
            if (pickWhat === "jump") {
                component.jumpMove = command;
            } else if (pickWhat === "shake") {
                component.shakeMove = command;
            }
            pickACommand.close();
        }
    }
}
