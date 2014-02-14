import QtQuick 2.0
import QtQuick.Dialogs 1.1

MessageDialog {
    id: removeContactDialog

    title: "Remove " + contact.nickname
    text: "Do you want to permanently remove " + contact.nickname + "?"
    informativeText: "This contact will no longer be able to message you, and will be notified " +
                     "about the removal. They may choose to send a new connection request."
    standardButtons: StandardButton.Yes | StandardButton.No
    onYes: contact.deleteContact()
}
