import Qt 4.7

Rectangle {
    height: 41
    gradient: Gradient {
        GradientStop { position: 0; color: "#bdbdbd"; }
        GradientStop { position: 1; color: "#959595"; }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "#7a7a7a"
    }

    Row {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8
        anchors.topMargin: 4
        anchors.bottomMargin: 5 /* one for the border line that is (incorrectly) counted */

        spacing: 7

        Avatar {
            id: avatar
            height: parent.height
            width: height

            //source:
        }

        Column {
            Text {
                id: nickname
                font.pixelSize: 15
                style: Text.Raised
                styleColor: "#c0c0c0"

                text: userIdentity.nickname
            }

            Text {
                id: contactId
                font.pixelSize: 10
                font.family: "Courier New"
                color: "#737373"
                text: userIdentity.contactID
                style: Text.Raised
                styleColor: "#a6a6a6"
            }
        }
    }
}
