import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ScrollView {
    id: scroll

    property alias model: messageView.model

    data: Rectangle {
        anchors.fill: scroll
        z: -1
        color: palette.base
    }

    ListView {
        id: messageView
        spacing: 8

        header: Item { width: 1; height: messageView.spacing }
        footer: Item { width: 1; height: messageView.spacing }
        delegate: MessageDelegate { }

        verticalLayoutDirection: ListView.BottomToTop
    }
}

