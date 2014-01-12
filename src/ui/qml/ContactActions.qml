import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.1
import "ContactWindow.js" as ContactWindow

Item {
    id: contactMenu

    property QtObject contact

    function openWindow() {
        var window = ContactWindow.getWindow(contact)
        window.requestActivate()
    }

    function removeContact() {
        removeContactDialog.open()
    }

    function openContextMenu() {
        contextMenu.popup()
    }

    signal renameTriggered

    Menu {
        id: contextMenu

        MenuItem {
            text: "Chat..."
            onTriggered: openWindow()
        }
        MenuSeparator { }
        MenuItem { text: "Get Info" }
        MenuItem { text: "Copy ID to Clipboard" }
        MenuSeparator { }
        MenuItem {
            text: "Rename"
            onTriggered: renameTriggered()
        }
        MenuItem {
            text: "Remove"
            onTriggered: removeContact()
        }
    }

    MessageDialog {
        id: removeContactDialog

        title: "Remove " + contact.nickname
        text: "Do you want to permanently remove " + contact.nickname + "?"
        informativeText: "This contact will no longer be able to message you, and will be notified " +
                         "about the removal. They may choose to send a new connection request."
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: contact.deleteContact()
    }
}

