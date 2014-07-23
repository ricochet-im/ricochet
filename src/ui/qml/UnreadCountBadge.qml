import QtQuick 2.0
import QtQuick.Controls 1.0

Rectangle {
    id: badge
    radius: 4
    width: number.width ? Math.max(height, number.width + 4) : 0
    height: number.height ? number.height + 4 : 0
    color: "#d80000"

    property int value

    Label {
        id: number
        anchors.centerIn: parent
        font.pointSize: styleHelper.pointSize - 2
        color: "white"
        font.bold: true
        text: value > 0 ? (value + "") : ""
    }
}

