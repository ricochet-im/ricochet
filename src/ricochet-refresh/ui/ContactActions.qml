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

    function sendFile() {
        contact.sendFile();
    }

    function canExportConversation() {
        return contact.conversation.conversationEventCount > 0;
    }

    function exportConversation() {
        if (contact.exportConversation() != true) {
            exportConversationFailedDialog.visible = true;
        }
    }

    signal renameTriggered

    MessageDialog {
        id: exportConversationFailedDialog

        title: qsTr("Warning")
        icon: StandardIcon.Warning
        text: qsTr("Could not successfully export conversation")
        standardButtons: StandardButton.Ok

        visible: false

        onAccepted: visible = false;
    }

    Menu {
        id: contextMenu

        /* QT automatically sets Accessible.text to MenuItem.text */
        MenuItem {
            //: Context menu command to open the chat screen in a separate window
            text: qsTr("Open Window")
            onTriggered: openWindow()
        }
        MenuItem {
            //: Context menu command to open a window showing the selected contact's details
            text: qsTr("Details...")
            onTriggered: openPreferences()
        }
        MenuItem {
            //: Context menu command to rename the selected contact
            text: qsTr("Rename")
            onTriggered: renameTriggered()
        }
        MenuItem {
            //: Context menu command to initiate a file transfer, opens a system file dialog
            text: qsTr("Send File...")
            onTriggered: sendFile()
        }
        MenuItem {
            //: Context menu command to initiate a chat log export, opens a system file dialog to export to
            enabled: canExportConversation()   // only enable if we have anything to export
            text: qsTr("Export Conversation...")
            onTriggered: exportConversation()
        }
        MenuSeparator { }
        MenuItem {
            //: Context menu command to remove a contact from the contact list
            text: qsTr("Remove")
            onTriggered: removeContact()
        }
    }

    Accessible.role: Accessible.List
    //: Description of the items in the context menu for accessibility tech like screen readers
    Accessible.name: qsTr("Contact options")

    Loader {
        id: removeContactDialog
        source: "MessageDialogWrapper.qml"
        active: false

        Accessible.role: Accessible.Window
    }
}

