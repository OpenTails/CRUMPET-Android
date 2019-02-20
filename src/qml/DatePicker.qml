import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import Qt.labs.calendar 1.0
import QtQuick.Controls.Material 2.4

Popup {
    id: control

    property date selectedDate: new Date()
    property int displayMonth: selectedDate.getMonth()
    property int displayYear: selectedDate.getFullYear()
    property int calendarWidth: parent.width * 0.85
    property int calendarHeight: parent.height * 0.8
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

    x: (parent.width - calendarWidth) / 2
    y: (parent.height - calendarHeight) / 2

    implicitWidth: calendarWidth
    implicitHeight: calendarHeight

    topPadding: 0
    leftPadding: 0
    rightPadding: 0

    closePolicy: Popup.CloseOnPressOutside

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

                Label {
                    text: displayYear
                    font.pointSize: 18
                    topPadding: 12
                    leftPadding: 24
                    opacity: 0.8
                }

                Label {
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

                Label {
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

            delegate: Label {
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

            delegate: Label {
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

            delegate: Label {
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
                text: qsTr("Today")
                Layout.fillWidth: true

                onClicked: selectedDate = new Date()
            }

            FlatButton {
                text: qsTr("Cancel")
                Layout.fillWidth: true

                onClicked: close()
            }

            FlatButton {
                text: qsTr("OK")
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
