/*
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
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

    parent: QQC2.Overlay.overlay
    showCloseButton: true
    title: i18nc("Title for the time picker", "Pick a Time")
    GridLayout {
        columns: 2
        rows: 4

        QQC2.Label {
            text: i18nc("Label for the Hours field on a time picker", "Hours")
        }

        QQC2.SpinBox {
            id: spinHours
            from: is12Hours ? 1 : 0
            to: is12Hours ? 12 : 23
            value: is12Hours ? (hours === 0 ? 12 : (hours === 12 ? 12 : hours % 12)) : hours
            Layout.fillWidth: true
        }

        QQC2.Label {
            text: i18nc("Label for the Minutes field on a time picker", "Minutes")
        }

        QQC2.SpinBox {
            id: spinMinutes
            from: 0
            to: 59
            value: minutes
            Layout.fillWidth: true
        }

        RowLayout {
            visible: is12Hours

            Layout.column: 1
            Layout.row: 2

            QQC2.RadioButton {
                id: radioAm
                text: i18nc("Label for the AM option on a AM/PM selector of a time picker", "AM")
                checked: is12Hours && hours < 12
            }

            QQC2.RadioButton {
                id: radioPm
                text: i18nc("Label for the PM option on a AM/PM selector of a time picker", "PM")
                checked: is12Hours && hours >= 12
            }
        }

        RowLayout {
            Layout.column: 0
            Layout.row: 3
            Layout.columnSpan: 2
            Layout.fillWidth: true

            FlatButton {
                text: i18nc("Button for resetting a time field to the current time, on a time picker", "Now")
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
                text: i18nc("Button for cancelling the time entry, on a time picker", "Cancel")
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth

                onClicked: close()
            }

            FlatButton {
                text: i18nc("Button for confirming the time entry, on a time picker", "OK")
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
