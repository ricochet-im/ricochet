import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

Item {
    anchors.fill: parent

    ContactList {
        id: contacts
        width: 200
        frameVisible: true
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            margins: 8
        }
    }

    ContactActions {
        id: contactActions
        contact: contacts.selectedContact
    }

    GridLayout {
        id: contactInfo
        columns: 2
        anchors {
            left: contacts.right
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            margins: 8
        }

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
            readOnly: true
            text: contactInfo.contact.contactID
        }

        Label { text: "Date added:" }
        Label {
            text: Qt.formatDate(contactInfo.contact.readSetting("whenCreated"), Qt.DefaultLocaleLongDate)
        }

        Label { text: "Last seen:"; visible: lastSeen.visible }
        Label {
            id: lastSeen
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

