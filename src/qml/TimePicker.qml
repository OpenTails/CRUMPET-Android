import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

Popup {
    id: control

    readonly property bool is12Hours: locale.amText

    property int hours: 0
    property int minutes: 0
    property var acceptedCallback: null

    signal accepted(var hours, var minutes)

    function showTimePicker(hours, minutes, acceptedCallback) {
        control.hours = hours
        control.minutes = minutes
        control.acceptedCallback = acceptedCallback

        updateControls()

        open()
    }

    function updateControls() {
        spinHours.value = is12Hours ? (hours === 0 ? 12 : (hours === 12 ? 12 : hours % 12)) : hours
        spinMinutes.value = minutes

        radioAm.checked = is12Hours && hours < 12
        radioPm.checked = is12Hours && hours >= 12
    }

    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    ColumnLayout {
        GridLayout {
            columns: 2
            rows: 3

            Label {
                text: qsTr("Hours")
            }

            SpinBox {
                id: spinHours
                from: is12Hours ? 1 : 0
                to: is12Hours ? 12 : 23
                value: is12Hours ? (hours === 0 ? 12 : (hours === 12 ? 12 : hours % 12)) : hours
            }

            Label {
                text: qsTr("Minutes")
            }

            SpinBox {
                id: spinMinutes
                from: 0
                to: 59
                value: minutes
            }

            RowLayout {
                visible: is12Hours

                Layout.column: 1
                Layout.row: 2

                RadioButton {
                    id: radioAm
                    text: qsTr("AM")
                    checked: is12Hours && hours < 12
                }

                RadioButton {
                    id: radioPm
                    text: qsTr("PM")
                    checked: is12Hours && hours >= 12
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            FlatButton {
                text: qsTr("Now")
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth

                onClicked: {
                    var now = new Date()
                    hours = now.getHours()
                    minutes = now.getMinutes()

                    updateControls()
                }
            }

            FlatButton {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth

                onClicked: close()
            }

            FlatButton {
                text: qsTr("OK")
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth

                onClicked: {
                    hours = is12Hours
                            ? (radioAm.checked ? (spinHours.value === 12
                                                  ? 0
                                                  : spinHours.value)
                                               : (spinHours.value === 12
                                                  ? 12
                                                  : spinHours.value + 12))
                            : spinHours.value

                    minutes = spinMinutes.value

                    accepted(hours, minutes)

                    if (acceptedCallback) {
                        var currentAcceptedCallback = acceptedCallback
                        acceptedCallback = null
                        currentAcceptedCallback(hours, minutes)
                    }

                    close()
                }
            }
        }
    }
}
