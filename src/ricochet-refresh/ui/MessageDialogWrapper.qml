import QtQuick 2.0
import QtQuick.Dialogs 1.1
import "utils.js" as Utils

MessageDialog {
    id: removeContactDialog

    //: A command to remove a contact from the contact list, %1 is the contact's nickname
    title: contact != null ? qsTr("Remove %1").arg(Utils.htmlEscaped(contact.nickname)) : ""
    //: Confirmation of contact removal, %1 is the contact's nickname
    text: contact != null ? qsTr("Do you want to permanently remove %1?").arg(Utils.htmlEscaped(contact.nickname)) : ""
    //: Message describing what will happen when a contact is removed from you contact list
    informativeText: qsTr("This contact will no longer be able to message you, and will be notified about the removal. They may choose to send a new connection request.")
    standardButtons: StandardButton.Yes | StandardButton.No
    onYes: contact.deleteContact()
}
