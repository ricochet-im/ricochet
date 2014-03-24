import QtQuick 2.0
import QtQuick.Controls 1.0
import org.torsionim.torsion 1.0

Column {
    id: delegate
    width: parent.width

    Loader {
        active: model.section === "offline"
        sourceComponent: Label {
            //: %1 nickname
            text: qsTr("%1 is offline").arg(contact !== null ? contact.nickname : "")
            width: background.parent.width
            elide: Text.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            color: palette.mid

            Rectangle {
                id: line
                width: (parent.width - parent.contentWidth) / 2 - 4
                height: 1
                y: (parent.height - 1) / 2
                color: Qt.lighter(palette.mid, 1.4)
            }

            Rectangle {
                width: line.width
                height: 1
                y: line.y
                x: parent.width - width
                color: line.color
            }
        }
    }

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
            width: Math.min(implicitWidth, background.__maxWidth)
            height: contentHeight
            x: (parent.width - width) / 2
            y: 6

            renderType: Text.NativeRendering
            selectionColor: palette.highlight
            selectedTextColor: palette.highlightedText
            font.pointSize: styleHelper.pointSize

            wrapMode: TextEdit.Wrap
            readOnly: true
            selectByMouse: true
            text: model.text
        }
    }
}
