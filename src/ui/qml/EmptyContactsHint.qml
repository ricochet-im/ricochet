import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Rectangle {
    id: emptyContactsHint
    x: -width + (targetButton.width / 2) + 15
    y: parent.height + 14
    width: text.contentWidth + 8
    height: text.contentHeight + 8
    color: "#c4e7ff"

    property Item targetButton
    property int maximumWidth

    Rectangle {
        rotation: 45
        width: 10
        height: 10
        x: parent.width - 20
        y: -5
        color: parent.color
    }

    Label {
        id: text
        text: qsTr("Click to add contacts")
        wrapMode: Text.Wrap
        width: maximumWidth - 16
        x: 4
        y: 4
    }
}

