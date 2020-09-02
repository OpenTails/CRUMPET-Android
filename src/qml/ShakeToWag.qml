import QtQuick 2.7
import QtQuick.Controls 2.4
import QtQuick.Extras 1.4
import QtSensors 5.12
import org.kde.kirigami 2.13 as Kirigami

Kirigami.ScrollablePage {
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

        amplitudeGyroscopeData.text = "Amplitude gyroscope: x: " + Math.round(amplitudeX) + ", y: " + Math.round(amplitudeY) + ", z: " + Math.round(amplitudeZ);

        if (maxAmpl - minAmpl > 700) {
            return "Jump";
        } else {
            return "Shake";
        }
    }

    ToggleButton {
        id: goButton;
        text: qsTr("Start");
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: function() {
            isReadingMode = goButton.checked;
            goButton.text = qsTr(isReadingMode ? "Stop": "Start");

            if (isReadingMode) {
                movementStatus.text = qsTr("make a move");
                gyroscope_x_values = [];
                gyroscope_y_values = [];
                gyroscope_z_values = [];
            } else {
                movementStatus.text = qsTr("Analyzing...");
                var movement = recognizeMovement();
                if (movement == "Jump") {
                    BTConnectionManager.sendMessage("TAILU2", []);
                } else if (movement == "Shake") {
                    BTConnectionManager.sendMessage("TAILU1", []);
                }

                movementStatus.text = "Your movement" + '\n' + "was recognized" + '\n' + "like a " + movement;
            }
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter

        Text {
            id: gyroscopeData
            text: "Gyroscope: x: 0, y: 0, z: 0";
        }

        Text {
            id: amplitudeGyroscopeData
            text: "Amplitude Gyroscope: x: 0, y: 0, z: 0";
        }

        Text {
            id: vectorChangesData
            text: "Vector Changes: x: 0, y: 0, z: 0";
        }

        Text {
            id: movementStatus;
            text: qsTr("Press start");
            font.pointSize: 36;
            anchors.horizontalCenter: parent.horizontalCenter
        }
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

                gyroscopeData.text = "Gyroscope: x: " + Math.round(x) + ", y: " + Math.round(y) + ", z: " + Math.round(z);
            }
        }
    }
}
