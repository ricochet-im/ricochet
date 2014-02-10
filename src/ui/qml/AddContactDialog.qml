import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: addContactWindow
    width: 400
    height: 300
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height
    flags: Qt.Dialog
    modality: Qt.WindowModal

    function close() {
        visible = false
    }

    function accept() {
        if (!fields.hasValidRequest)
            return

        userIdentity.contacts.createContactRequest(fields.contactId.text, fields.name.text, "", fields.message.text)
        close()
    }

    ColumnLayout {
        id: infoArea
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: 8
            leftMargin: 16
            rightMargin: 16
        }

        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Qt.AlignHCenter
            wrapMode: Text.Wrap
            text: "Share your Torsion ID to allow connection requests"
            font.pointSize: 12
        }

        ContactIDField {
            id: localId
            Layout.fillWidth: true
            readOnly: true
            text: userIdentity.contactID
            horizontalAlignment: Qt.AlignHCenter
        }

        Item { height: 1 }

        Rectangle {
            color: palette.mid
            height: 1
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        Item { height: 1 }
    }

    ContactRequestFields {
        id: fields
        anchors {
            left: parent.left
            right: parent.right
            top: infoArea.bottom
            bottom: buttonRow.top
            margins: 8
            leftMargin: 16
            rightMargin: 16
        }

        Component.onCompleted: contactId.focus = true
    }

    RowLayout {
        id: buttonRow
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: 16
            bottomMargin: 8
        }

        Button {
            text: "Cancel"
            onClicked: addContactWindow.close()
        }

        Button {
            text: "Add"
            isDefault: true
            enabled: fields.hasValidRequest
            onClicked: addContactWindow.accept()
        }
    }
}

