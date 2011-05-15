import Qt 4.7

PopoutItem {
    id: contactPage

    itemIsFromLoader: false

    property QtObject contact

    Rectangle {
        id: contactHeader
        anchors.top: contactPage.top
        anchors.left: contactPage.left
        anchors.right: contactPage.right
        height: 97

        gradient: Gradient {
            GradientStop { position: 0; color: "#eeeeee"; }
            GradientStop { position: 1; color: "#d3d3d3"; }
        }

        Avatar {
            id: avatar
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 8
            width: height

            source: contact.avatarPath
        }

        Column {
            anchors.left: avatar.right
            anchors.leftMargin: 8
            anchors.top: avatar.top

            Text {
                font.bold: true
                font.pixelSize: 13
                text: contact.nickname
            }

            Text {
                color: "#a5a5a5"
                text: contact.contactID
                font.family: "Courier New"
                font.pixelSize: 10
            }
        }

        Image {
            id: popoutButton
            anchors.right: contactHeader.right
            anchors.rightMargin: 3
            anchors.top: contactHeader.top
            anchors.topMargin: 2
            source: "popout.png"
            opacity: 0.3

            PopoutClickArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true

                target: contactPage
            }

            states: State {
                name: "mouseOver"
                when: mouseArea.containsMouse || mouseArea.windowPopped

                PropertyChanges {
                    target: popoutButton
                    opacity: 1.0
                }
            }

            transitions: Transition {
                to: "mouseOver"
                reversible: true

                NumberAnimation {
                    target: popoutButton
                    property: "opacity"
                    duration: 300
                }
            }
        }

        MouseArea {
            anchors.verticalCenter: contactHeader.bottom
            anchors.left: contactHeader.left
            anchors.right: contactHeader.right
            height: 10

            onPositionChanged: {
                contactHeader.height = Math.max(48, Math.min(mouse.y+contactHeader.height, 97))
            }
        }
    }

    Rectangle {
        id: headerSeparator
        anchors.bottom: contactHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "#989898"
    }

    ChatArea {
        id: chat

        anchors.top: headerSeparator.bottom
        anchors.left: contactPage.left
        anchors.right: contactPage.right
        anchors.bottom: contactPage.bottom
    }
}
