import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

ScrollView {
    id: scroll

    data: [
        Rectangle {
            anchors.fill: scroll
            z: -1
            color: palette.base
        },
        ContactsModel {
            id: contactsModel
            identity: userIdentity
        }
    ]

    property QtObject selectedContact
    property ListView view: contactListView

    // Emitted for double click on a contact
    signal contactActivated(ContactUser contact, Item actions)

    onSelectedContactChanged: {
        if (selectedContact !== contactsModel.contact(contactListView.currentIndex)) {
            contactListView.currentIndex = contactsModel.rowOfContact(selectedContact)
        }
    }

    ListView {
        id: contactListView
        model: contactsModel
        currentIndex: -1

        signal contactActivated(ContactUser contact, Item actions)
        onContactActivated: scroll.contactActivated(contact, actions)

        onCurrentIndexChanged: {
            // Not using a binding to allow writes to selectedContact
            scroll.selectedContact = contactsModel.contact(contactListView.currentIndex)
        }

        data: [
            MouseArea {
                anchors.fill: parent
                z: -100
                onClicked: contactListView.currentIndex = -1
            }
        ]

        section.property: "status"
        section.delegate: Label {
            height: implicitHeight + 8
            width: parent.width
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter
            text: {
                switch (parseInt(section)) {
                    case ContactUser.Online: return qsTr("Online")
                    case ContactUser.Offline: return qsTr("Offline")
                    case ContactUser.RequestPending: return qsTr("Requests")
                    case ContactUser.RequestRejected: return qsTr("Rejected")
                    case ContactUser.Outdated: return qsTr("Outdated")
                }
            }
            font.bold: true
        }

        delegate: ContactListDelegate { }
    }
}
