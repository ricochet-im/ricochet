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
            retakeFocus.start()
        } else if (activeFocusItem === null) {
            inactive = true
        }
    }

    Timer {
        id: retakeFocus
        onTriggered: textInput.forceActiveFocus()
        interval: 1
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

            TextArea {
                id: textInput
                Layout.fillWidth: true
                y: 2
                // This ridiculous incantation enables an automatically sized TextArea
                Layout.preferredHeight: mapFromItem(flickableItem, 0, 0).y * 2 +
                                        Math.max(textHeight.height + 2*edit.textMargin, flickableItem.contentHeight)
                Layout.maximumHeight: (textHeight.height * 4) + (2 * edit.textMargin)
                textMargin: 3
                wrapMode: TextEdit.Wrap

                property TextEdit edit

                Label { id: textHeight; visible: false }

                Component.onCompleted: {
                    var objects = contentItem.contentItem.children
                    for (var i = 0; i < objects.length; i++) {
                        if (objects[i].hasOwnProperty('textDocument')) {
                            edit = objects[i]
                            break
                        }
                    }

                    edit.Keys.pressed.connect(keyHandler)
                }

                function keyHandler(event) {
                    switch (event.key) {
                        case Qt.Key_Enter:
                        case Qt.Key_Return:
                            if (!(event.modifiers & Qt.ShiftModifier)) {
                                send()
                                event.accepted = true
                                break
                            }
                        default:
                            event.accepted = false
                    }
                }

                function send() {
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

