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

    property var accelerometer_x_values: [];
    property var accelerometer_y_values: [];
    property var accelerometer_z_values: [];

    property var gyroscope_x_values: [];
    property var gyroscope_y_values: [];
    property var gyroscope_z_values: [];

    property bool isReadingMode: false;

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

                accelerometer_x_values = [];
                accelerometer_y_values = [];
                accelerometer_z_values = [];

                gyroscope_x_values = [];
                gyroscope_y_values = [];
                gyroscope_z_values = [];
            } else {
                movementStatus.text = qsTr("Analyzing...");

                var minX = 0;
                var maxX = 0;
                var minY = 0;
                var maxY = 0;
                var minZ = 0;
                var maxZ = 0;

                var minAmpl = 0;
                var maxAmpl = 0;

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
                    movementStatus.text = "Your movement" + '\n' + "was recognized" + '\n' + "like a Jump";
                } else {
                    movementStatus.text = "Your movement" + '\n' + "was recognized" + '\n' + "like a Shake";
                }

                minX = 0;
                maxX = 0;
                minY = 0;
                maxY = 0;
                minZ = 0;
                maxZ = 0;

                count = accelerometer_x_values.length;
                for (var i = 0; i < count; i++) {
                    if (accelerometer_x_values[i] <= minX) {
                        minX = accelerometer_x_values[i];
                    } else {
                        maxX = accelerometer_x_values[i];
                    }

                    if (accelerometer_y_values[i] < minY) {
                        minY = accelerometer_y_values[i];
                    } else {
                        maxY = accelerometer_y_values[i];
                    }

                    if (accelerometer_z_values[i] < minZ) {
                        minZ = accelerometer_z_values[i];
                    } else {
                        maxZ = accelerometer_z_values[i];
                    }
                }

                amplitudeX = maxX - minX;
                amplitudeY = maxY - minY;
                amplitudeZ = maxZ - minZ;

                minAmpl = Math.min(amplitudeX, amplitudeY, amplitudeZ);
                maxAmpl = Math.max(amplitudeX, amplitudeY, amplitudeZ);

                amplitudeAccelerometerData.text = "Amplitude accelerometer: x: " + Math.round(amplitudeX) + ", y: " + Math.round(amplitudeY) + ", z: " + Math.round(amplitudeZ);
            }

            //todo: trigger a bluetooth command
            //BTConnectionManager.sendMessage("TAILHM", []);
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            id: accelerometerData
            text: "Accelerometer: x: 0, y: 0, z: 0";
        }

        Text {
            id: gyroscopeData
            text: "Gyroscope: x: 0, y: 0, z: 0";
        }
        Text {
            id: amplitudeAccelerometerData
            text: "Amplitude Accelerometer: x: 0, y: 0, z: 0";
        }

        Text {
            id: amplitudeGyroscopeData
            text: "Amplitude Gyroscope: x: 0, y: 0, z: 0";
        }

        Text {
            id: movementStatus;
            text: qsTr("Press start");
            font.pointSize: 36;
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Accelerometer {
        id: accelerometer
        dataRate: 100
        active: true
        onReadingChanged: {
            if (isReadingMode) {
                var x = accelerometer.reading.x;
                var y = accelerometer.reading.y;
                var z = accelerometer.reading.z;

                accelerometer_x_values.push(x);
                accelerometer_y_values.push(y);
                accelerometer_z_values.push(z);

                console.log("x: ", x);
                console.log("y: ", y);
                console.log("z: ", z);

                accelerometerData.text = "Accelerometer: x: " + Math.round(x) + ", y: " + Math.round(y) + ", z: " + Math.round(z);
            }
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
