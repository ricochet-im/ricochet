import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

GridLayout {
    id: contactFields
    columns: 2

    property bool readOnly
    property ContactIDField contactId: contactIdField
    property TextField name: nameField
    property TextArea message: messageField
    property bool hasValidRequest: contactIdField.acceptableInput && nameField.text.length

    Label {
        //: Label for the contact id text box in the 'add new contact' window
        text: qsTr("ID:")
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        Accessible.role: Accessible.StaticText
        Accessible.name: text
    }

    ContactIDField {
        id: contactIdField
        Layout.fillWidth: true
        readOnly: contactFields.readOnly
        showCopyButton: false
    }

    Label {
        //: Label for the contact nickname text box in the 'add new contact' window
        text: qsTr("Name:")
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
        Accessible.role: Accessible.StaticText
        Accessible.name: text
    }

    TextField {
        id: nameField
        Layout.fillWidth: true
        readOnly: contactFields.readOnly

        Accessible.role: Accessible.Dialog
        Accessible.name: text
        //: Description of textbox for setting a contact's nickname for accessibility tech like screen readers
        Accessible.description: qsTr("Field for the contact's nickname")
    }

    Label {
        //: Label for the contact greeting message text box in the 'add new contact' window
        text: qsTr("Message:")
        Layout.alignment: Qt.AlignTop | Qt.AlignRight
        Accessible.role: Accessible.StaticText
        Accessible.name: text
    }

    TextArea {
        id: messageField
        Layout.fillWidth: true
        Layout.fillHeight: true
        textFormat: TextEdit.PlainText
        readOnly: contactFields.readOnly
        Accessible.role: Accessible.Dialog
        Accessible.name: text
        //: Description of textbox for setting a new contact's initial greeting message for accessibility tech like screen readers
        Accessible.description: qsTr("Field for the contact's greeting message")
    }
}
