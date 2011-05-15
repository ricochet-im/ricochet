import org.torsionim.torsion 1.0
import Qt 4.7

Item {
    id: contactListItem
    height: 40
    width: ListView.view.width
    clip: true

    Avatar {
        id: avatar
        x: 17
        anchors.verticalCenter: parent.verticalCenter
        sourceSize: Qt.size(32, 32)

        source: avatarPath
    }

    Text {
        id: nickText
        anchors.left: avatar.right
        anchors.leftMargin: 10
        anchors.verticalCenter: avatar.verticalCenter
        text: nickname

        color: (status == ContactUser.Online) ? "black" : "#aeaeae"
        style: (status == ContactUser.Online) ? Text.Normal : Text.Raised
        styleColor: "#f2f2f2"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: { ListView.view.currentIndex = index; setCurrentContact(contact, contactListItem) }
    }

    Component.onCompleted: {
        if (currentContact == contact) {
            ListView.view.currentIndex = index
            setCurrentContact(contact, contactListItem)
        }
    }

    states: State {
        name: "current"
        when: contactListItem == currentContactItem

        PropertyChanges {
            target: nickText
            color: "white"
            style: Text.Raised
            styleColor: "#777777"
        }
    }
}
