/*
 *   Copyright 2019 Ildar Gilmanov <gil.ildar@gmail.com>
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
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.11
import Qt.labs.calendar 1.0
import QtQuick.Controls.Material 2.4
import org.kde.kirigami 2.13 as Kirigami

Kirigami.OverlaySheet {
    id: control

    property date selectedDate: new Date()
    property int displayMonth: selectedDate.getMonth()
    property int displayYear: selectedDate.getFullYear()
    property int calendarWidth: root.width * 0.85
    property int calendarHeight: root.height * 0.8
    property var acceptedCallback: null

    signal accepted(var date)

    function showDatePicker(date, acceptedCallback) {
        control.selectedDate = date
        control.acceptedCallback = acceptedCallback
        open()
    }

    onSelectedDateChanged: {
        displayMonth = selectedDate.getMonth()
        displayYear = selectedDate.getFullYear()
    }

    Item {
        width: calendarWidth
        height: calendarHeight

        implicitWidth: calendarWidth
        implicitHeight: calendarHeight

        GridLayout {
            columns: 2
            rows: 5
            anchors.fill: parent

            Rectangle {
                color: Material.primary
                Material.foreground: "white"

                implicitHeight: headerLayout.implicitHeight
                implicitWidth: headerLayout.implicitWidth

                Layout.columnSpan: 2
                Layout.column: 0
                Layout.row: 0
                Layout.fillWidth: true

                ColumnLayout {
                    id: headerLayout

                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    QQC2.Label {
                        text: displayYear
                        font.pointSize: 18
                        topPadding: 12
                        leftPadding: 24
                        opacity: 0.8
                    }

                    QQC2.Label {
                        text: Qt.formatDate(selectedDate, "ddd, d, MMM")

                        font.pointSize: 20
                        leftPadding: 24
                        bottomPadding: 12
                    }
                }
            }

            ColumnLayout {
                spacing: 6

                Layout.columnSpan: 2
                Layout.column: 0
                Layout.row: 1
                Layout.fillWidth: true

                RowLayout {
                    height: implicitHeight * 2
                    spacing: 6

                    FlatButton {
                        text: "<"

                        Layout.fillWidth: true
                        Layout.preferredWidth: 1

                        onClicked: {
                            if(displayMonth > 0) {
                                displayMonth--
                            } else {
                                displayMonth = 11
                                displayYear--
                            }
                        }
                    }

                    QQC2.Label {
                        text: monthGrid.title
                        font.pointSize: 18
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        Layout.fillWidth: true
                        Layout.preferredWidth: 3
                    }

                    FlatButton {
                        text: ">"

                        Layout.fillWidth: true
                        Layout.preferredWidth: 1

                        onClicked: {
                            if(displayMonth < 11) {
                                displayMonth++
                            } else {
                                displayMonth = 0
                                displayYear++
                            }
                        }
                    }
                }
            }

            DayOfWeekRow {
                id: dayOfWeekRow

                font.bold: false
                rightPadding: 24

                Layout.column: 1
                Layout.row: 2
                Layout.fillWidth: true

                delegate: QQC2.Label {
                    text: model.shortName
                    font: dayOfWeekRow.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            WeekNumberColumn {
                id: weekNumbers

                visible: false
                month: displayMonth
                year: displayYear

                font.bold: false
                leftPadding: 24

                Layout.column: 0
                Layout.row: 3
                Layout.fillHeight: true

                delegate: QQC2.Label {
                    text: model.weekNumber
                    font: weekNumbers.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            MonthGrid {
                id: monthGrid

                month: displayMonth
                year: displayYear

                Layout.column: 1
                Layout.row: 3
                Layout.fillHeight: true
                Layout.fillWidth: true
                rightPadding: 24

                delegate: QQC2.Label {
                    readonly property bool isSelected: model.day === selectedDate.getDate()
                                                       && model.month === selectedDate.getMonth()
                                                       && model.year === selectedDate.getFullYear()

                    text: model.day
                    opacity: model.month === monthGrid.month ? 1 : 0.5
                    color: pressed || isSelected ? "white" : (model.today ? Material.accent : Material.foreground)
                    font.bold: model.today ? true : false
                    fontSizeMode: Text.Fit
                    minimumPointSize: 8

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    background: Rectangle {
                        width: parent.font.pixelSize * 2
                        height: width
                        radius: width / 2
                        color: Material.primary
                        visible: pressed || parent.isSelected
                        anchors.centerIn: parent
                    }
                }

                onClicked: selectedDate = date
            }

            RowLayout {
                Layout.column: 0
                Layout.columnSpan: 2
                Layout.row: 4
                Layout.fillWidth: true

                FlatButton {
                    text: i18nc("Button for selecting the date of today, in a date picker", "Today")
                    Layout.fillWidth: true

                    onClicked: selectedDate = new Date()
                }

                FlatButton {
                    text: i18nc("Button for cancelling the selection of a date, in a date picker", "Cancel")
                    Layout.fillWidth: true

                    onClicked: close()
                }

                FlatButton {
                    text: i18nc("Button for confirming the selection of a date, in a date picker", "OK")
                    Layout.fillWidth: true

                    onClicked: {
                        accepted(selectedDate)

                        if (acceptedCallback) {
                            var currentAcceptedCallback = acceptedCallback
                            acceptedCallback = null
                            currentAcceptedCallback(selectedDate)
                        }

                        control.close()
                    }
                }
            }
        }
    }
}
