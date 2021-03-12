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
        section.delegate: Row {
            width: parent.width - x
            height: label.height + 4
            x: 8
            spacing: 6

            Label {
                id: label
                y: 2

                font.pointSize: styleHelper.pointSize
                font.bold: true
                font.capitalization: Font.SmallCaps
                textFormat: Text.PlainText
                color: "#3f454a"

                text: {
                    // Translation strings are uppercase for legacy reasons, and because they
                    // should correctly be capitalized. We go lowercase only because it looks
                    // nicer when using SmallCaps, and that's a display detail.
                    switch (parseInt(section)) {
                        //: Section header in the contact list for users which are online
                        case ContactUser.Online: return qsTr("Online").toLowerCase()
                        //: Section header in the contact list for users which are offline
                        case ContactUser.Offline: return qsTr("Offline").toLowerCase()
                        //: Section header in the contact list for users requesting to be added to the user's contact list
                        case ContactUser.RequestPending: return qsTr("Requests").toLowerCase()
                        //: Section header in the contact list for users that have rejected the user's request to be added to their contact list
                        case ContactUser.RequestRejected: return qsTr("Rejected").toLowerCase()
                    }
                }

                Accessible.role: Accessible.StaticText
                Accessible.name: text
                //: Description of the section header for accessibility tech like screen readers
                Accessible.description: qsTr("Status for the given contacts")
            }

            Rectangle {
                height: 1
                width: parent.width - x
                anchors {
                    top: label.verticalCenter
                    topMargin: 1
                }

                color: "black"
                opacity: 0.1
            }
        }

        delegate: ContactListDelegate { }
    }
}
