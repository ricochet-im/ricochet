import org.torsionim.torsion 1.0
import Qt 4.7
import QtDesktop 0.1

TopLevelWindow {
    id: window
    sheet: true

    Keys.onEscapePressed: close()

    Rectangle {
        id: dialog
        SystemPalette { id: palette }
        color: palette.window
        width: 400
        height: 220

        property int margin: 8

        Column {
            id: labels
            spacing: dialog.margin
            width: nicknameLabel.paintedWidth
            x: dialog.margin
            y: dialog.margin

            Text {
                text: "ID:"
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                height: contactId.height
                width: labels.width
            }

            Text {
                id: nicknameLabel
                text: "Nickname:"
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                height: contactNickname.height
                width: labels.width
            }
        }

        Column {
            anchors.left: labels.right
            anchors.right: dialog.right
            anchors.top: dialog.top
            anchors.bottom: dialog.bottom
            anchors.margins: dialog.margin
            spacing: labels.spacing

            TextField {
                id: contactId
                width: parent.width
                focus: true
                placeholderText: "abcdefghijklmnop@Torsion"
                validator: ContactIDValidator {
                    notContactOfIdentity: userIdentity

                    onContactExists: {
                        errorMessage.text = "You already have <b>" + contact.nickname + "</b> as a contact!"
                        errorMessage.state = "display"
                    }
                }
                KeyNavigation.tab: contactNickname

                Component.onCompleted: {
                    forceActiveFocus()
                    checkClipboard()
                }

                Connections {
                    target: helper
                    onClipboardTextChanged: contactId.checkClipboard()
                }

                function checkClipboard() {
                    var idRegex = /(^([a-z2-7]{16})$|[a-z2-7]{16}@Torsion)/i
                    var clipboard = helper.clipboardText.match(idRegex)
                    if (clipboard !== null) {
                        contactId.text = clipboard[0]
                        if (contactId.activeFocus)
                            contactNickname.forceActiveFocus()
                    }
                }
            }

            TextField {
                id: contactNickname
                width: parent.width

                validator: NicknameValidator {
                    uniqueToIdentity: userIdentity
                }

                KeyNavigation.tab: message
                KeyNavigation.backtab: contactId
            }
        }

        Text {
            id: errorMessage
            anchors.centerIn: message
            color: "#990000"
            horizontalAlignment: Qt.AlignHCenter
            opacity: 0

            Connections {
                target: contactId
                onTextChanged: errorMessage.state = ""
            }

            states: State {
                name: "display"

                PropertyChanges {
                    target: errorMessage
                    opacity: 1
                }

                PropertyChanges {
                    target: message
                    opacity: 0
                }
            }

            transitions: Transition {
                to: "display"
                reversible: true

                NumberAnimation {
                    target: errorMessage
                    property: "opacity"
                    duration: 150
                }

                NumberAnimation {
                    target: message
                    property: "opacity"
                    duration: 100
                }
            }
        }

        TextArea {
            id: message

            anchors.top: labels.bottom
            anchors.left: dialog.left
            anchors.right: dialog.right
            anchors.bottom: buttons.top
            anchors.margins: dialog.margin
            anchors.topMargin: dialog.margin*2

            textFormat: TextEdit.PlainText

            tabChangesFocus: true
            KeyNavigation.priority: KeyNavigation.BeforeItem
            KeyNavigation.tab: okBtn
            KeyNavigation.backtab: contactNickname
        }

        MouseArea { /* Workaround QTCOMPONENTS-848 */
            anchors.fill: message
            onPressed: { message.forceActiveFocus(); mouse.accepted = false }
        }

        Text {
            anchors.fill: message
            anchors.topMargin: 4
            color: "darkgray"
            horizontalAlignment: Qt.AlignHCenter
            opacity: (!message.text.length && !message.activeFocus) ? message.opacity : 0
            clip: true

            text: "Enter a message for the contact request\n\n" +
                  "Tell " + (contactNickname.text.length ? contactNickname.text : "your contact") +
                  " who you are or why they should accept"

            Behavior on opacity { NumberAnimation { duration: 90 } }
        }

        Row {
            id: buttons
            anchors.bottom: dialog.bottom
            anchors.right: dialog.right
            anchors.margins: dialog.margin
            anchors.rightMargin: 1

            Button {
                id: cancelBtn
                text: "Cancel"
                KeyNavigation.tab: okBtn
                KeyNavigation.backtab: message

                onClicked: dialog.parent.close()
            }

            Button {
                id: okBtn
                text: "Add Contact"
                defaultbutton: true
                enabled: contactId.acceptableInput && contactNickname.acceptableInput &&
                         message.text.length > 0
                KeyNavigation.backtab: message

                onClicked: {
                    var contact = userIdentity.contacts.createContactRequest(contactId.text,
                                                                             contactNickname.text,
                                                                             "" /* myNickname */,
                                                                             message.text)

                    if (contact === null) {
                        errorMessage.text = "Something went wrong.\n\nYou should restart Torsion and try again."
                        errorMessage.state = "display"
                        return
                    }

                    contactList.realSetCurrentContact(contact)
                    dialog.parent.close()
                }
            }
        }
    }
}
