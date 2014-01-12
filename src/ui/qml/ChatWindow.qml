import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

ApplicationWindow {
    id: chatWindow
    width: 500
    height: 400
    title: contact !== null ? contact.nickname : ""

    property var contact
    signal closed

    onVisibleChanged: {
        if (!visible)
            closed()
    }

    ConversationModel {
        id: conversationModel
        contact: chatWindow.contact
    }

    ChatMessageArea {
        anchors.fill: parent
        model: conversationModel
    }

    statusBar: StatusBar {
        Item {
            width: parent.width
            height: input.height + 4

            TextField {
                id: input
                width: parent.width
                y: 2

                onAccepted: {
                    conversationModel.sendMessage(input.text)
                    input.text = ""
                }
            }
        }
    }

    Connections {
        target: contact
        onIncomingChatMessage: {
            chatWindow.alert(0)
        }
    }
}

