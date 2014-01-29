import QtQuick 2.0
import org.torsionim.torsion 1.0

MouseArea {
    id: messageDelegate
    width: parent.width
    height: background.height + 4

    Rectangle {
        id: background
        width: Math.max(30, child.width + 10)
        height: child.height + 10
        x: model.isOutgoing ? parent.width - width - 10 : 10

        property int __maxWidth: parent.width * 0.8

        color: (model.status === ConversationModel.Error) ? "#ffdcc4" : "#c4e7ff"
        Behavior on color { ColorAnimation { } }

        Rectangle {
            rotation: 45
            width: 10
            height: 10
            x: model.isOutgoing ? parent.width - 20 : 10
            y: model.isOutgoing ? parent.height - 5 : -5
            color: parent.color
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            opacity: (model.status === ConversationModel.Sending || model.status === ConversationModel.Error) ? 1 : 0
            visible: opacity > 0
            color: Qt.lighter(parent.color, 1.15)

            Behavior on opacity { NumberAnimation { } }
        }

        Item {
            id: child
            width: childrenRect.width
            height: childrenRect.height
            anchors.centerIn: parent

            TextEdit {
                id: textField
                wrapMode: Text.Wrap
                width: Math.min(implicitWidth, background.__maxWidth)
                height: paintedHeight
                readOnly: true
                selectByMouse: true
                text: model.text
            }
        }
    }
}
