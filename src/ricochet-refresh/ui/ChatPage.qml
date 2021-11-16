import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

FocusScope {
    id: chatPage

    property ContactUser contact
    property TextArea textField: textInput
    property var conversationModel: (contact !== null) ? contact.conversation : null

    function forceActiveFocus() {
        textField.forceActiveFocus()
    }

    onVisibleChanged: if (visible) forceActiveFocus()

    property bool active: visible && activeFocusItem !== null
    onActiveChanged: {
        if (active)
            conversationModel.resetUnreadCount()
    }

    Connections {
        target: conversationModel
        function onUnreadCountChanged(user, unreadCount) {
            if (active) conversationModel.resetUnreadCount()
        }
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
            // 1 = Status::Offline
            status: contact != null ? contact.status : 1
        }

        Label {
            text: contact != null ? contact.nickname : ""
            textFormat: Text.PlainText
            font.pointSize: styleHelper.pointSize
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
            bottom: statusBar.top
        }
        model: conversationModel
    }

    StatusBar {
        id: statusBar
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
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
                                        Math.max(styleHelper.textHeight + 2*edit.textMargin, flickableItem.contentHeight)
                Layout.maximumHeight: (styleHelper.textHeight * 4) + (2 * edit.textMargin)
                textMargin: 3
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                font.pointSize: styleHelper.pointSize
                focus: true

                property TextEdit edit

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
                            if (event.modifiers & Qt.ShiftModifier || event.modifiers & Qt.AltModifier) {
                                textInput.insert(textInput.cursorPosition, "\n")
                            } else {
                                send()
                            }
                            event.accepted = true
                            break
                        default:
                            event.accepted = false
                    }
                }

                function send() {
                    if (textInput.length > 2000)
                        textInput.remove(2000, textInput.length)
                    conversationModel.sendMessage(textInput.text)
                    textInput.remove(0, textInput.length)
                }

                onLengthChanged: {
                    if (textInput.length > 2000)
                        textInput.remove(2000, textInput.length)
                }

                Accessible.role: Accessible.EditableText
                //: label for accessibility tech like screen readers
                Accessible.name: qsTr("Message area") // todo: translation
                //: description of the text area used to send messages for accessibility tech like screen readers
                Accessible.description: qsTr("Write the message to be sent here. Press enter to send")
            }
        }
    }
}

