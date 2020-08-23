import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.1
import "ContactWindow.js" as ContactWindow

Item {
    id: contactMenu

    property QtObject contact

    function openWindow() {
        var window = ContactWindow.getWindow(contact)
        window.raise()
        window.requestActivate()
    }

    function removeContact() {
        removeContactDialog.active = true
        if (removeContactDialog.item !== null) {
            removeContactDialog.item.open()
        } else if (uiMain.showRemoveContactDialog(contact)) {
            contact.deleteContact()
        }
    }

    function openContextMenu() {
        contextMenu.popup()
    }

    function openPreferences() {
        root.openPreferences("ContactPreferences.qml", { 'selectedContact': contact })
    }

    signal renameTriggered

    Menu {
        id: contextMenu

        MenuItem {
            text: qsTr("Open Window")
            onTriggered: openWindow()
        }
        MenuItem {
            text: qsTr("Details...")
            onTriggered: openPreferences()
        }
        MenuItem {
            text: qsTr("Rename")
            onTriggered: renameTriggered()
        }
        MenuSeparator { }
        MenuItem {
            text: qsTr("Remove")
            onTriggered: removeContact()
        }
    }

    Loader {
        id: removeContactDialog
        source: "MessageDialogWrapper.qml"
        active: false
    }
}

