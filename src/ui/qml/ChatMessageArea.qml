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
        }
    ]

    ListView {
        id: messageView
        spacing: 12

        header: Item { width: 1; height: messageView.spacing }
        footer: Item { width: 1; height: messageView.spacing }
        delegate: MessageDelegate { }

        verticalLayoutDirection: ListView.BottomToTop
    }
}

