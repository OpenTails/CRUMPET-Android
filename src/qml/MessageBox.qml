import QtQuick 2.11
import QtQuick.Controls 2.4 as QQC2
import org.kde.kirigami 2.6 as Kirigami

Kirigami.OverlaySheet {
    id: control;

    property alias headerText: textHeader.text;
    property alias text: label.text;
    property var okHandler: null;

    function showMessageBox(header, text, okHandler) {
        control.headerText = header;
        control.text = text;
        control.okHandler = okHandler;

        open();
    }

    header: Kirigami.Heading {
        id: textHeader;
    }

    Column {
        width: root.width - Kirigami.Units.largeSpacing * 4;
        spacing: Kirigami.Units.smallSpacing;

        QQC2.Label {
            id: label;
            width: parent.width;
            wrapMode: Text.Wrap;
        }

        QQC2.Button {
            text: qsTr("Ok");
            highlighted: true;
            width: parent.width;

            onClicked: {
                if (okHandler) {
                    handler = okHandler;
                    okHandler = null;
                    close();
                    handler();
                } else {
                    close();
                }
            }
        }
    }
}
