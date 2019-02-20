import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Popup {
    id: control

    property alias text: label.text
    property var okHandler: null

    function showMessageBox(text, okHandler) {
        control.text = text
        control.okHandler = okHandler

        open()
    }

    modal: true
    closePolicy: Popup.NoAutoClose

    width: Math.min(root.width * 0.8, layout.implicitWidth)

    ColumnLayout {
        id: layout
        anchors.fill: parent

        Label {
            id: label
            wrapMode: Text.Wrap

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Button {
            text: qsTr("Ok")
            highlighted: true
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                if (okHandler) {
                    handler = okHandler
                    okHandler = null
                    close()
                    handler()
                } else {
                    close()
                }
            }
        }
    }
}
