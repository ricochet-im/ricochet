import QtQuick 2.0
import QtQuick.Dialogs 1.1

MessageDialog {
    id: removeContactDialog

    title: qsTr("Remove %1").arg(contact.nickname)
    //: %1 nickname
    text: qsTr("Do you want to permanently remove %1?").arg(contact.nickname)
    informativeText: qsTr("This contact will no longer be able to message you, and will be notified about the removal. They may choose to send a new connection request.")
    standardButtons: StandardButton.Yes | StandardButton.No
    onYes: contact.deleteContact()
}
