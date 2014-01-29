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

    property bool inactive: true
    onActiveFocusItemChanged: {
        // Focus text input when window regains focus
        if (activeFocusItem !== null && inactive) {
            inactive = false
            textInput.forceActiveFocus()
        } else if (activeFocusItem === null) {
            inactive = true
        }
    }

    ConversationModel {
        id: conversationModel
        contact: chatWindow.contact
    }

    RowLayout {
        id: infoBar
        anchors {
            top: parent.top
            left: parent.left
            leftMargin: 4
            right: parent.right
            rightMargin: 4
        }
        height: implicitHeight + 8
        spacing: 8

        PresenceIcon {
            status: contact.status
        }

        Label {
            text: contact.nickname
        }

        Item {
            Layout.fillWidth: true
            height: 1
        }
    }

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            top: infoBar.top
            bottom: infoBar.bottom
        }
        color: palette.base
        z: -1

        Column {
            anchors {
                top: parent.bottom
                left: parent.left
                right: parent.right
            }
            Rectangle { width: parent.width; height: 1; color: palette.midlight; }
            Rectangle { width: parent.width; height: 1; color: palette.window; }
        }
    }

    ChatMessageArea {
        anchors {
            top: infoBar.bottom
            topMargin: 2
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        model: conversationModel
    }

    statusBar: StatusBar {
        height: statusLayout.height + 8
        RowLayout {
            id: statusLayout
            width: statusBar.width - 8
            y: 2

            TextField {
                id: textInput
                Layout.fillWidth: true
                y: 2

                onAccepted: {
                    conversationModel.sendMessage(textInput.text)
                    textInput.text = ""
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

