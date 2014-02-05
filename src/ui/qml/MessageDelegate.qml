import QtQuick 2.0
import org.torsionim.torsion 1.0

Rectangle {
    id: background
    width: Math.max(30, textField.width + 12)
    height: textField.height + 12
    x: model.isOutgoing ? parent.width - width - 10 : 10

    property int __maxWidth: parent.width * 0.8

    color: (model.status === ConversationModel.Error) ? "#ffdcc4" : ( model.isOutgoing ? "#eaeced" : "#c4e7ff" )
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

    TextEdit {
        id: textField
        wrapMode: TextEdit.Wrap
        width: Math.min(implicitWidth, background.__maxWidth)
        height: contentHeight
        readOnly: true
        selectByMouse: true
        text: model.text
        x: (parent.width - width) / 2
        y: 6
    }
}
