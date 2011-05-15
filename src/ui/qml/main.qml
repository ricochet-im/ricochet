import org.torsionim.torsion 1.0
import Qt 4.7

Rectangle {
    id: window
    width: 360
    height: 360
    focus: true

    TorsionToolBar {
        id: toolBar
        anchors.top: window.top
        anchors.left: window.left
        anchors.right: window.right
    }

    ContactList {
        id: contactList
        anchors.top: toolBar.bottom
        anchors.left: window.left
        anchors.bottom: window.bottom

        onCurrentContactChanged: {
            if (currentContact !== null)
            {
                pageArea.setCurrentPage(currentContact)
                pageArea.currentItem.contact = currentContact
            }
        }
    }

    PageSwitcher {
        id: pageArea

        delegate: ContactPage {
            anchors.left: contactList.right
            anchors.top: toolBar.bottom
            anchors.right: window.right
            anchors.bottom: window.bottom
            visible: PageSwitcherBase.isCurrentItem || state == "windowed"
        }

    }

    Keys.onPressed: {
        if (event.key == Qt.Key_O)
        {
            contactList.currentContact.status = Number(!contactList.currentContact.status)
            event.accepted = true
        }
    }
}
