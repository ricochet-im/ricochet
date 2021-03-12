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
        //: Error status string displayed when tor daemon does not launch successfully
        text: qsTr("The Tor process was not started successfully. This is most likely an installation or system error.")
        font.bold: true
        wrapMode: Text.Wrap

        Accessible.role: Accessible.StaticText
        Accessible.name: text
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

    TorLogDisplay {
        id: logDisplay
        width: parent.width
        height: text.length > 0 ? 300 : 0
    }

    RowLayout {
        anchors {
            left: parent.left
            right: parent.right
            margins: 8
        }

        Item { height: 1; Layout.fillWidth: true }
        Button {
            //: Button title to quit/terminate the program
            text: qsTr("Quit")
            onClicked: Qt.quit()

            Accessible.role: Accessible.Button
            Accessible.name: text
            Accessible.onPressAction: {
                Qt.quit()
            }
        }
    }
}

