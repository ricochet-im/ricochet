import org.torsionim.torsion 1.0
import Qt 4.7

Item {
    Item {
        id: textArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: textInputArea.top

        Component.onCompleted: {
            helper.createTextEdit(textArea)
        }
    }

    Rectangle {
        id: textInputArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: textInput.height + 9

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: "#989898"
        }

        Rectangle {
            anchors.fill: parent
            anchors.topMargin: 1
            color: "#f2f2f2"
        }

        TextInput {
            id: textInput
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 1
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.right: parent.right
            anchors.rightMargin: 4
            z: 1
        }

        MouseArea {
            anchors.fill: parent
            onClicked: textInput.focus = true
        }
    }
}
