import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

Item {
    property alias selectedContact: contacts.selectedContact

    RowLayout {
        anchors {
            fill: parent
            margins: 8
        }

        ContactList {
            id: contacts
            Layout.preferredWidth: 200
            Layout.minimumWidth: 150
            Layout.fillHeight: true
            frameVisible: true
        }

        data: [
            ContactActions {
                id: contactActions
                contact: contacts.selectedContact
            }
        ]

        ColumnLayout {
            id: contactInfo
            visible: contact !== null
            Layout.fillHeight: true
            Layout.fillWidth: true

            property QtObject contact: contacts.selectedContact
            property QtObject request: (contact !== null) ? contact.contactRequest : null

            Item { height: 1; width: 1 }
            Label {
                id: nickname
                Layout.fillWidth: true
                text: visible ? contactInfo.contact.nickname : ""
                textFormat: Text.PlainText
                horizontalAlignment: Qt.AlignHCenter
                font.pointSize: styleHelper.pointSize + 1

                property bool renameMode
                property Item renameItem
                onRenameModeChanged: {
                    if (renameMode && renameItem === null) {
                        renameItem = renameComponent.createObject(nickname)
                        renameItem.forceActiveFocus()
                        renameItem.selectAll()
                    } else if (!renameMode && renameItem !== null) {
                        renameItem.focus = false
                        renameItem.visible = false
                        renameItem.destroy()
                        renameItem = null
                    }
                }

                MouseArea { anchors.fill: parent; onDoubleClicked: nickname.renameMode = true }

                Component {
                    id: renameComponent

                    TextField {
                        id: nameField
                        anchors {
                            left: parent.left
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                        text: contactInfo.contact.nickname
                        horizontalAlignment: nickname.horizontalAlignment
                        font.pointSize: nickname.font.pointSize
                        onEditingFinished: {
                            contactInfo.contact.nickname = text
                            nickname.renameMode = false
                        }
                    }
                }
            }
            Item { height: 1; width: 1 }

            ContactIDField {
                Layout.fillWidth: true
                Layout.minimumWidth: 100
                readOnly: true
                text: visible ? contactInfo.contact.contactID : ""
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2

                Label { text: qsTr("Date added:"); Layout.alignment: Qt.AlignRight }
                Label {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    text: visible ? Qt.formatDate(contactInfo.contact.settings.read("whenCreated"), Qt.DefaultLocaleLongDate) : ""
                    textFormat: Text.PlainText
                }

                Label { text: qsTr("Last seen:"); visible: lastSeen.visible; Layout.alignment: Qt.AlignRight }
                Label {
                    id: lastSeen
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    visible: contactInfo.request === null
                    text: visible ? Qt.formatDate(contactInfo.contact.settings.read("lastConnected"), Qt.DefaultLocaleLongDate) : ""
                    textFormat: Text.PlainText
                }

                Label { text: qsTr("Request:"); visible: requestStatus.visible; Layout.alignment: Qt.AlignRight }
                Label {
                    id: requestStatus
                    visible: contactInfo.request !== null
                    textFormat: Text.PlainText
                    text: {
                        var re = ""
                        if (contactInfo.request === null)
                            return re
                        switch (contactInfo.request.status) {
                            case OutgoingContactRequest.Pending: re = qsTr("Pending connection"); break
                            case OutgoingContactRequest.Acknowledged: re = qsTr("Delivered"); break
                            case OutgoingContactRequest.Accepted: re = qsTr("Accepted"); break
                            case OutgoingContactRequest.Error: re = qsTr("Error"); break
                            case OutgoingContactRequest.Rejected: re = qsTr("Rejected"); break
                        }
                        if (contactInfo.request.isConnected) {
                            //: %1 status, e.g. "Accepted"
                            re = qsTr("%1 (Connected)").arg(re)
                        }
                        return re
                    }
                }

                Label { text: qsTr("Response:"); visible: rejectMessage.visible; Layout.alignment: Qt.AlignRight }
                Label {
                    id: rejectMessage
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    text: visible ? contactInfo.request.rejectMessage : ""
                    textFormat: Text.PlainText
                    visible: (contactInfo.request !== null) && (contactInfo.request.rejectMessage !== "")
                }
            }

            Item { height: 1; width: 1 }
            Rectangle {
                color: palette.mid
                height: 1
                Layout.fillWidth: true
            }
            Item { height: 1; width: 1 }

            RowLayout {
                Layout.fillWidth: true

                Button {
                    text: qsTr("Rename")
                    onClicked: nickname.renameMode = !nickname.renameMode
                }

                Item { Layout.fillWidth: true; height: 1 }

                Button {
                    text: qsTr("Remove")
                    onClicked: contactActions.removeContact()
                }
            }

            Item {
                Layout.fillHeight: true
                width: 1
            }
        }
    }
}
