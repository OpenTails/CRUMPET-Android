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
                accelerometer_x_values = [];
                accelerometer_y_values = [];
                accelerometer_z_values = [];

                gyroscope_x_values = [];
                gyroscope_y_values = [];
                gyroscope_z_values = [];
            } else {
                //todo: analyze values
            }

            //todo: trigger a bluetooth command
            //BTConnectionManager.sendMessage("TAILHM", []);
        }
    }

    Column {
        Text {
            id: accelerometerData
            text: "Accelerometer: x: 0, y: 0, z: 0";
        }

        Text {
            id: gyroscopeData
            text: "Gyroscope: x: 0, y: 0, z: 0";
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
