import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

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

    property QtObject selectedContact: contactsModel.contact(contactListView.currentIndex)
    property ListView view: contactListView

    ListView {
        id: contactListView

        model: contactsModel

        section.property: "status"
        section.delegate: Label {
            height: implicitHeight + 8
            width: parent.width
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter
            text: {
                switch (parseInt(section)) {
                    case ContactUser.Online: return "Online"
                    case ContactUser.Offline: return "Offline"
                    case ContactUser.RequestPending: return "Requested"
                }
            }
            font.pointSize: 12
            font.bold: true
        }

        delegate: ContactListDelegate { }
    }
}
