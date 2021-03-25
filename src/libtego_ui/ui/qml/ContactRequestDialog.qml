import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: contactRequestDialog
    width: 640
    height: 200
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height
    flags: styleHelper.dialogWindowFlags
    modality: Qt.WindowModal
    title: mainWindow.title

    signal closed
    onVisibleChanged: if (!visible) closed()

    property QtObject request
    property bool hasValidContact: request != null && request.hostname != "" && fields.name.text.length

    function close() {
        visible = false
    }

    function accept() {
        request.nickname = fields.name.text
        request.accept()
        close()
    }

    function reject() {
        request.reject()
        close()
    }

    GridLayout {
        id: infoArea
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: 8
            leftMargin: 16
            rightMargin: 16
        }
        columns: 2

        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Qt.AlignHCenter
            wrapMode: Text.Wrap
            //: Descriptive text that is displayed in a popup window when a user tries to add you as a contact
            text: qsTr("Someone new is asking to connect to you")
            Accessible.role: Accessible.PopupMenu
            Accessible.name: text
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
        readOnly: true

        Component.onCompleted: {
            contactId.text = request.contactId
            name.text = request.nickname
            name.readOnly = false
            name.focus = true
            message.text = request.message
        }
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
            //: Label for button which rejects a contact request when pressed
            text: qsTr("Reject")
            onClicked: contactRequestDialog.reject()
            Accessible.role: Accessible.Button
            Accessible.name: text
            //: Description of what 'Reject' button does for accessibility tech like screen readers
            Accessible.description: qsTr("Rejects the incoming contact request")
        }

        Button {
            //: Label for button which accepts a contact request when pressed
            text: qsTr("Accept")
            enabled: hasValidContact
            onClicked: contactRequestDialog.accept()
            Accessible.role: Accessible.Button
            Accessible.name: text
            //: Description of what 'Accept' button does for accessibility tech like screen readers
            Accessible.description: qsTr("Accepts the incoming contact request")
        }
    }

    Action {
        shortcut: StandardKey.Close
        onTriggered: contactRequestDialog.close()
    }

    Action {
        shortcut: "Escape"
        onTriggered: contactRequestDialog.close()
    }
}

