import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Column {
    id: statusPage
    spacing: 8

    property bool hasError: torInstance.hasError

    Label {
        anchors {
            left: parent.left
            right: parent.right
            margins: 8
        }

        text: "The Tor process was not started successfully. This is most likely an installation or system error."
        font.bold: true
        wrapMode: Text.Wrap
    }

    Label {
        anchors {
            left: parent.left
            right: parent.right
            margins: 8
        }

        text: torInstance.errorMessage
        wrapMode: Text.Wrap
    }

    RowLayout {
        anchors {
            left: parent.left
            right: parent.right
            margins: 8
        }

        Item { height: 1; Layout.fillWidth: true }
        Button {
            text: "Quit"
            onClicked: Qt.quit()
        }
    }
}

