import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import QtQuick.Controls.Material 2.4
import QtGraphicalEffects 1.0

Button {
    id: control

    property alias textColor: controlText.color
    property alias textAlignment: controlText.horizontalAlignment

    focusPolicy: Qt.NoFocus

    leftPadding: 6
    rightPadding: 6

    Layout.fillWidth: true
    Layout.preferredWidth: 1

    contentItem: Text {
        id: controlText

        text: control.text
        color: Material.foreground
        opacity: enabled ? 1.0 : 0.3
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.capitalization: Font.AllUppercase
        font.weight: Font.Medium
    }

    background: Rectangle {
        color: control.pressed ? controlText.color : "transparent"
        opacity: control.pressed ? 0.12 : 1.0
        radius: 2
        implicitHeight: 48
        Layout.minimumWidth: 88
    }
}
