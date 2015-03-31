import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ColumnLayout {
    anchors {
        fill: parent
        margins: 8
    }

    CheckBox {
        text: qsTr("Use a single window for conversations")
        checked: uiSettings.data.combinedChatWindow || false
        onCheckedChanged: {
            uiSettings.write("combinedChatWindow", checked)
        }
    }

    CheckBox {
        text: qsTr("Open links in default browser without prompting")
        checked: uiSettings.data.alwaysOpenBrowser || false
        onCheckedChanged: {
            uiSettings.write("alwaysOpenBrowser", checked)
        }
    }

    CheckBox {
        text: qsTr("Show timestamp on each chat message")
        checked: uiSettings.data.alwaysShowTimestamps || false
        onCheckedChanged: {
            uiSettings.write("alwaysShowTimestamps", checked)
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
}
