import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

RowLayout {
    id: contactId
    property alias text: field.text
    property alias readOnly: field.readOnly
    property alias horizontalAlignment: field.horizontalAlignment
    property alias acceptableInput: field.acceptableInput
    property bool showCopyButton: true

    TextField {
        id: field
        Layout.fillWidth: true
        font.family: "Courier"
        validator: readOnly ? null : idValidator

        ContactIDValidator {
            id: idValidator
            notContactOfIdentity: userIdentity
        }

        MouseArea {
            anchors.fill: parent
            enabled: field.readOnly
            onClicked: {
                field.selectAll()
                field.copy()
            }
        }
    }

    Button {
        text: qsTr("Copy")
        visible: contactId.showCopyButton
        onClicked: {
            field.selectAll()
            field.copy()
        }
    }
}

