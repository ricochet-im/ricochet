import org.torsionim.torsion 1.0
import Qt 4.7

Rectangle {
    id: contactList

    width: 187
    color: "#d4d8da"
    clip: true

    property QtObject currentContact
    property Item currentContactItem

    function setCurrentContact(contact, contactItem) {
        currentContactItem = contactItem
        currentContact = contact
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: "#989898"
    }

    ListView {
        id: contactsView
        anchors.fill: parent
        anchors.topMargin: 2
        anchors.bottomMargin: 6
        spacing: 3

        model: contactsModel
        delegate: Qt.createComponent("ContactListGroup.qml")

        highlight: highlightDelegate
        highlightFollowsCurrentItem: false
    }

    Component {
        id: highlightDelegate

        Rectangle {
            id: highlight
            width: contactsView.width - 8
            x: currentContactItem.x + 7
            /* Strange usage of the comma operator used to create a binding on the y property */
            y: currentContactItem.y, currentContactItem.ListView.view.y,
               currentContactItem.mapToItem(parent, 0, 0).y
            height: currentContactItem.height

            Behavior on y {
                SmoothedAnimation {
                    velocity: 400
                }
            }

            gradient: Gradient {
                GradientStop { position: 0; color: "#a2c3da"; }
                GradientStop { position: 1; color: "#718898"; }
            }

            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: 1
                anchors.topMargin: 1
                anchors.bottomMargin: 1

                gradient: Gradient {
                    GradientStop { position: 0; color: "#d2dce5"; }
                    GradientStop { position: 1; color: "#8ca4ba"; }
                }
            }
        }
    }
}
