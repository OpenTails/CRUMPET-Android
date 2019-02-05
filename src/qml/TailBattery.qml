import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.4 as Kirigami
import org.thetailcompany.digitail 1.0

Item {
    visible: height > 0;
    height: BTConnectionManager.isConnected ? batteryLabel.height : 0;

    Label {
        id: batteryLabel;
        text: "Tail battery:"
        height: Kirigami.Units.iconSizes.small;
    }

    Row {
        spacing: Kirigami.Units.smallSpacing;
        anchors.right: parent.right;

        Repeater {
            model: 4;

            Rectangle {
                height: Kirigami.Units.iconSizes.small;
                width: height;
                radius: height / 2;
                color: "transparent";

                border {
                    width: 1;
                    color: "black";
                }

                anchors.verticalCenter: parent.verticalCenter;

                Rectangle {
                    visible: modelData < BTConnectionManager.batteryLevel;
                    height: parent.height - Kirigami.Units.smallSpacing * 2;
                    width: height;
                    radius: height / 2;
                    color: "black";
                    anchors.centerIn: parent;
                }
            }
        }
    }
}
