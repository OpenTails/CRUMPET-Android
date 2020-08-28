import QtQuick 2.7
import QtQuick.Controls 2.4
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

    Button {
        id: goButton;
        text: qsTr("Go");
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: function() {
            //todo: trigger a bluetooth command
            //BTConnectionManager.sendMessage("TAILHM", []);
        }
    }

    Text {
        id: accelerometerData
        text: "x: 0, y: 0, z: 0";
    }

    Accelerometer {
        id: accelerometer
        dataRate: 100
        active:true
        onReadingChanged: {
            console.log("x: ", accelerometer.reading.x);
            console.log("y: ", accelerometer.reading.y);
            console.log("z: ", accelerometer.reading.z);

            accelerometerData.text = "x: " + Math.round(accelerometer.reading.x) +
                                   ", y: " + Math.round(accelerometer.reading.y) +
                                   ", z: " + Math.round(accelerometer.reading.z);
        }
    }
}
