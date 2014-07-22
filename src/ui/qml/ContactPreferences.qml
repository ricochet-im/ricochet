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

        GridLayout {
            id: contactInfo
            visible: contact !== null
            columns: 2
            Layout.fillHeight: true
            Layout.fillWidth: true

            property QtObject contact: contacts.selectedContact
            property QtObject request: (contact !== null) ? contact.contactRequest : null

            Label { text: qsTr("Nickname:") }
            TextField {
                Layout.fillWidth: true
                text: visible ? contactInfo.contact.nickname : ""
                onAccepted: contactInfo.contact.nickname = text
            }

            Label { text: qsTr("ID:") }
            ContactIDField {
                Layout.fillWidth: true
                Layout.minimumWidth: 100
                readOnly: true
                text: visible ? contactInfo.contact.contactID : ""
            }

            Label { text: qsTr("Date added:") }
            Label {
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: visible ? Qt.formatDate(contactInfo.contact.settings.read("whenCreated"), Qt.DefaultLocaleLongDate) : ""
            }

            Label { text: qsTr("Last seen:"); visible: lastSeen.visible }
            Label {
                id: lastSeen
                Layout.fillWidth: true
                elide: Text.ElideRight
                visible: contactInfo.request === null
                text: visible ? Qt.formatDateTime(contactInfo.contact.settings.read("lastConnected"), Qt.DefaultLocaleLongDate) : ""
            }

            Label { text: qsTr("Request:"); visible: requestStatus.visible }
            Label {
                id: requestStatus
                visible: contactInfo.request !== null
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

            Label { text: qsTr("Response:"); visible: rejectMessage.visible }
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
                text: qsTr("Remove")
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
