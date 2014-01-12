import QtQuick 2.0
import QtQuick.Controls 1.0

Rectangle {
    id: delegate
    color: highlighted ? palette.highlight : palette.base
    width: parent.width
    height: nameLabel.height + 8

    property bool highlighted: ListView.isCurrentItem

    PresenceIcon {
        id: presenceIcon
        anchors {
            left: parent.left
            leftMargin: 8
            verticalCenter: parent.verticalCenter
        }
        status: model.status
    }

    Label {
        id: nameLabel
        y: 4
        anchors {
            left: presenceIcon.right
            leftMargin: 8
            right: parent.right
            rightMargin: 8
        }
        text: model.name
        elide: Text.ElideRight
        color: highlighted ? palette.highlightedText : palette.text
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
                contextMenu.openWindow()
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

