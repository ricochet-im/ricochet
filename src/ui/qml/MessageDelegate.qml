import QtQuick 2.0

MouseArea {
    id: messageDelegate
    width: parent.width
    height: background.height

    Rectangle {
        id: background
        width: Math.max(30, child.width + 10)
        height: child.height + 10
        x: model.isOutgoing ? parent.width - width - 10 : 10

        property int __maxWidth: parent.width * 0.8

        color: "#0099ff"

        Rectangle {
            transform: Rotation { angle: 45 }
            width: 14
            height: 14
            color: parent.color
            x: model.isOutgoing ? parent.width - width : width
            y: model.isOutgoing ? parent.height - height : -(height / 2)
        }

        Item {
            id: child
            width: childrenRect.width
            height: childrenRect.height
            anchors.centerIn: parent

            TextEdit {
                id: textField
                wrapMode: Text.Wrap
                color: "white"
                width: Math.min(implicitWidth, background.__maxWidth)
                height: paintedHeight
                readOnly: true
                selectByMouse: true
                text: model.text
            }
        }
    }
}
