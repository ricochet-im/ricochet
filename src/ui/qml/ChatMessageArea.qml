import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ScrollView {
    id: scroll

    property alias model: messageView.model

    data: [
        Rectangle {
            anchors.fill: scroll
            z: -1
            color: palette.base
        },
        // Workaround to a ScrollView bug causing content to not appear on Linux
        Binding {
            target: __verticalScrollBar
            property: "enabled"
            value: false
            when: messageView.contentHeight < messageView.height
        },
        // Workaround #76 - Fixed properly by Qt change-id I2a19e4c70096259ef7bbb5a00727ac767c4b8c57
        Connections {
            target: scroll.scroller
            onRecursionGuardChanged: {
                var scroller = scroll.scroller
                var flickableItem = scroll.flickableItem
                var viewport = scroll.viewport

                // Starting layout; update maximumValue manually
                if (scroller.recursionGuard) {
                    scroller.verticalScrollBar.maximumValue = flickableItem.contentHeight > viewport.height ? flickableItem.originY + flickableItem.contentHeight - viewport.height + scroll.__viewTopMargin : 0
                }
            }
        }
    ]

    // Other part of workaround for #76
    // Will be disabled when Qt change is applied because contentHeight property no longer exists
    property var scroller: scroll.__scroller && scroll.__scroller.hasOwnProperty('contentHeight') ? scroll.__scroller : null
    onScrollerChanged: {
        if (scroller !== null) {
            // Break binding
            scroller.verticalScrollBar.maximumValue = 0
        }
    }

    ListView {
        id: messageView
        spacing: 12
        pixelAligned: true

        header: Item { width: 1; height: messageView.spacing }
        footer: Item { width: 1; height: messageView.spacing }
        delegate: MessageDelegate { }

        verticalLayoutDirection: ListView.BottomToTop
    }
}

