import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Item {
    anchors.fill: parent

    TextArea {
        anchors {
            fill: parent
            margins: 8
        }
        readOnly: true
        text: uiMain.aboutText
    }
}

