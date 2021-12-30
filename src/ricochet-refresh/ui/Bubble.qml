import QtQuick 2.0
import QtQuick.Controls 1.0

Rectangle {
    id: bubble
    x: {
        switch (horizontalAlignment) {
            case Qt.AlignHCenter: return -width + (target.width / 2) + 15
            case Qt.AlignLeft: return 20
            case Qt.AlignRight: return (target.width - width - 20)
        }
    }
    y: parent.height + 14
    width: label.contentWidth + 12
    height: label.contentHeight + 12
    color: "#c4e7ff"
    border.color: Qt.darker(color, 1.2)
    border.width: 1
    opacity: displayed ? 1 : 0
    visible: opacity

    property alias textFormat: label.textFormat

    property Item target
    property int maximumWidth: target ? target.width : 100
    property int horizontalAlignment: Qt.AlignHCenter
    property bool displayed: text.length
    property alias text: label.text

    Behavior on opacity { NumberAnimation { duration: 200 } }

    Rectangle {
        id: arrow
        rotation: 45
        width: 10
        height: 10
        x: (horizontalAlignment == Qt.AlignLeft) ? 20 : parent.width - 20
        y: -5
        color: parent.color
        border.color: parent.border.color
        border.width: 1
    }

    Rectangle {
        x: arrow.x - 1
        width: arrow.width + 2
        height: 10
        color: parent.color
    }

    Label {
        id: label
        wrapMode: Text.Wrap
        width: maximumWidth - 16
        textFormat: Text.PlainText
        x: 6
        y: 6

        Accessible.name: text
        Accessible.role: Accessible.StaticText
    }
}

