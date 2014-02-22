import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

Item {

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

        GridLayout {
            id: contactInfo
            columns: 2
            Layout.fillHeight: true
            Layout.fillWidth: true

            property QtObject contact: contacts.selectedContact
            property QtObject request: (contact !== null) ? contact.contactRequest : null

            Label { text: "Nickname:" }
            TextField {
                Layout.fillWidth: true
                text: contactInfo.contact.nickname
                onAccepted: contactInfo.contact.nickname = text
            }

            Label { text: "ID:" }
            ContactIDField {
                Layout.fillWidth: true
                Layout.minimumWidth: 100
                readOnly: true
                text: contactInfo.contact.contactID
            }

            Label { text: "Date added:" }
            Label {
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: Qt.formatDate(contactInfo.contact.readSetting("whenCreated"), Qt.DefaultLocaleLongDate)
            }

            Label { text: "Last seen:"; visible: lastSeen.visible }
            Label {
                id: lastSeen
                Layout.fillWidth: true
                elide: Text.ElideRight
                visible: contactInfo.request === null
                text: Qt.formatDateTime(contactInfo.contact.readSetting("lastConnected"), Qt.DefaultLocaleLongDate)
            }

            Label { text: "Request:"; visible: requestStatus.visible }
            Label {
                id: requestStatus
                visible: contactInfo.request !== null
                text: {
                    var re = ""
                    if (contactInfo.request === null)
                        return re
                    switch (contactInfo.request.status) {
                        case OutgoingContactRequest.Pending: re = "Pending connection"; break
                        case OutgoingContactRequest.Acknowledged: re = "Delivered"; break
                        case OutgoingContactRequest.Accepted: re = "Accepted"; break
                        case OutgoingContactRequest.Error: re = "Error"; break
                        case OutgoingContactRequest.Rejected: re = "Rejected"; break
                    }
                    if (contactInfo.request.isConnected)
                        re += " (Connected)"
                    return re
                }
            }

            Label { text: "Response:"; visible: rejectMessage.visible }
            Label {
                id: rejectMessage
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: visible ? contactInfo.request.rejectMessage : ""
                visible: (contactInfo.request !== null) && (contactInfo.request.rejectMessage !== "")
            }

            Item { height: 1; width: 1 }
            Rectangle {
                color: palette.mid
                height: 1
                Layout.fillWidth: true
                Layout.columnSpan: 2
            }
            Item { height: 1; width: 1 }

            Button {
                text: "Remove"
                Layout.columnSpan: 2
                onClicked: contactActions.removeContact()
            }

            Item {
                Layout.fillHeight: true
                width: 1
            }
        }
    }
}
