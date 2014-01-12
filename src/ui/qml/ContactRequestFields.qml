import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

GridLayout {
    id: contactFields
    columns: 2

    property bool readOnly
    property TextField contactId: contactIdField
    property TextField name: nameField
    property TextArea message: messageField
    property bool hasValidRequest: contactIdField.acceptableInput && nameField.text.length

    Label {
        text: "ID:"
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
    }

    TextField {
        id: contactIdField
        Layout.fillWidth: true
        font.family: "Courier"
        validator: RegExpValidator { regExp: /[a-z2-7]{16}@Torsion/i }
        readOnly: contactFields.readOnly
    }

    Label {
        text: "Name:"
        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
    }

    TextField {
        id: nameField
        Layout.fillWidth: true
        readOnly: contactFields.readOnly
    }

    Label {
        text: "Message:"
        Layout.alignment: Qt.AlignTop | Qt.AlignRight
    }

    TextArea {
        id: messageField
        Layout.fillWidth: true
        Layout.fillHeight: true
        readOnly: contactFields.readOnly
    }
}
