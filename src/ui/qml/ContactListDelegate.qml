import QtQuick 2.0
import QtQuick.Controls 1.0
import im.ricochet 1.0

Rectangle {
    id: delegate
    color: highlighted ? "#c4e7ff" : "white"
    width: parent != null ? parent.width : 0
    height: nameLabel.height + 8

    property bool highlighted: ListView.isCurrentItem
    onHighlightedChanged: {
        if (renameMode)
            renameMode = false
    }

    PresenceIcon {
        id: presenceIcon
        anchors {
            left: parent.left
            leftMargin: 20
            verticalCenter: nameLabel.verticalCenter
        }
        status: model.status
    }

    Label {
        id: nameLabel
        anchors {
            left: presenceIcon.right
            leftMargin: 6
            right: unreadBadge.left
            rightMargin: 8
            verticalCenter: parent.verticalCenter
        }
        text: model.name
        textFormat: Text.PlainText
        elide: Text.ElideRight
        font.pointSize: styleHelper.pointSize
        color: "black"
        opacity: model.status === ContactUser.Online ? 1 : 0.8
    }

    UnreadCountBadge {
        id: unreadBadge
        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
            rightMargin: 8
        }

        value: model.contact.conversation.unreadCount
    }

    ContactActions {
        id: contextMenu
        contact: model.contact

        onRenameTriggered: renameMode = true
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: {
            if (!delegate.ListView.isCurrentItem)
                contactListView.currentIndex = model.index
        }

        onClicked: {
            if (mouse.button === Qt.RightButton) {
                contextMenu.openContextMenu()
            }
        }

        onDoubleClicked: {
            if (mouse.button === Qt.LeftButton) {
                contactListView.contactActivated(model.contact, contextMenu)
            }
        }
    }

    property bool renameMode
    property Item renameItem
    onRenameModeChanged: {
        if (renameMode && renameItem === null) {
            renameItem = renameComponent.createObject(delegate)
            renameItem.forceActiveFocus()
            renameItem.selectAll()
        } else if (!renameMode && renameItem !== null) {
            renameItem.visible = false
            renameItem.destroy()
            renameItem = null
        }
    }

    Component {
        id: renameComponent

        TextField {
            id: nameField
            anchors {
                left: nameLabel.left
                right: nameLabel.right
                verticalCenter: nameLabel.verticalCenter
            }
            text: model.contact.nickname
            onAccepted: {
                model.contact.nickname = text
                delegate.renameMode = false
            }
        }
    }
}

