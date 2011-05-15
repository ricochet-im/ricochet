import Qt 4.7

ListView {
    id: contactsGroupView
    spacing: 3
    height: 1 /* See autoSize() */
    width: contactsView.width
    interactive: false /* disable FLickable; scrolling takes place in the parent view */

    function autoSize() {
        /* Size the view to fit its contents exactly. 19 is the height of the header item. */
        height = (count ? contentHeight : 0) + 19
    }

    onContentHeightChanged: autoSize()
    onCountChanged: autoSize()

    Connections {
        target: contactsModel
        onRowsInserted: myModel.forceUpdateRoot()
        onRowsRemoved: myModel.forceUpdateRoot()
        onRowsMoved: myModel.forceUpdateRoot()
    }

    VisualDataModel {
        id: myModel
        model: contactsModel
        delegate: Qt.createComponent("ContactListItem.qml")

        function forceUpdateRoot() {
            rootIndex = parentModelIndex()
            rootIndex = modelIndex(index)
        }

        Component.onCompleted: {
            rootIndex = modelIndex(index)
        }
    }

    model: myModel

    header: groupHeader

    Component {
        id: groupHeader

        Item {
            width: ListView.view.width
            height: 19

            Text {
                id: title
                text: nickname
                x: 16
                y: 0
                color: "#6e7f94"
                font.pixelSize: 13
                font.bold: true
            }
        }
    }

    Connections {
        target: contactList
        onCurrentContactItemChanged: {
            if (currentContactItem !== currentItem)
                contactsGroupView.currentIndex = -1
        }
    }
}
