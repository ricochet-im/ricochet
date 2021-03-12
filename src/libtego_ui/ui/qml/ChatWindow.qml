import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

ApplicationWindow {
    id: chatWindow
    width: 500
    height: 400
    title: contact !== null ? contact.nickname : ""

    property alias contact: chatPage.contact
    signal closed

    onVisibleChanged: {
        if (!visible)
            closed()
    }

    onClosed: {
        // If not also in combined window mode, clear chat history when closing
        if (!uiSettings.data.combinedChatWindow)
            chatPage.conversationModel.clear()
    }

    property bool inactive: true
    onActiveFocusItemChanged: {
        // Focus text input when window regains focus
        if (activeFocusItem !== null && inactive) {
            inactive = false
            retakeFocus.start()
        } else if (activeFocusItem === null) {
            inactive = true
        }
    }

    ChatPage {
        id: chatPage
        anchors.fill: parent
    }

    Action {
        shortcut: StandardKey.Close
        onTriggered: chatWindow.close()
    }
}

