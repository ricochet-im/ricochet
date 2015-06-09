import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0
import "ContactWindow.js" as ContactWindow

ApplicationWindow {
    id: window
    title: "Ricochet"
    visibility: Window.AutomaticVisibility

    width: 250
    height: 400
    minimumHeight: 400
    minimumWidth: uiSettings.data.combinedChatWindow ? 650 : 250
    maximumWidth: uiSettings.data.combinedChatWindow ? (1 << 24) - 1 : 250

    onMinimumWidthChanged: width = Math.max(width, minimumWidth)
    onMaximumWidthChanged: width = Math.min(width, maximumWidth)

    // OS X Menu
    Loader {
        active: Qt.platform.os == 'osx'
        sourceComponent: MenuBar {
            Menu {
                title: "Ricochet"
                MenuItem {
                    text: qsTranslate("QCocoaMenuItem", "Preference")
                    onTriggered: toolBar.preferences.trigger()
                }
            }
        }
    }

    Connections {
        target: userIdentity.contacts
        onUnreadCountChanged: {
            if (unreadCount > 0) {
                audioNotification.playIncomingMessage()
                if (!uiSettings.data.combinedChatWindow) {
                    var cwindow = ContactWindow.getWindow(user)
                    cwindow.alert(0)
                } else if (!ContactWindow.windowExists(user))
                    window.alert(0)
            }
        }
        onContactStatusChanged: {
            if (status === ContactUser.Online) audioNotification.playContactOnline()
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ColumnLayout {
            spacing: 0
            Layout.preferredWidth: combinedChatView.visible ? 220 : 0
            Layout.fillWidth: !combinedChatView.visible

            MainToolBar {
                id: toolBar
                // Needed to allow bubble to appear over contact list
                z: 3
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                ContactList {
                    id: contactList
                    anchors.fill: parent
                    opacity: offlineLoader.item !== null ? (1 - offlineLoader.item.opacity) : 1

                    onContactActivated: {
                        if (contact.status === ContactUser.RequestPending || contact.status === ContactUser.RequestRejected) {
                            actions.openPreferences()
                        } else if (!uiSettings.data.combinedChatWindow) {
                            actions.openWindow()
                        }
                    }
                }

                Loader {
                    id: offlineLoader
                    active: torControl.torStatus !== TorControl.TorReady || (item !== null && item.visible)
                    anchors.fill: parent
                    source: Qt.resolvedUrl("OfflineStateItem.qml")
                }
            }
        }

        Rectangle {
            visible: combinedChatView.visible
            width: 1
            Layout.fillHeight: true
            color: Qt.darker(palette.window, 1.5)
        }

        PageView {
            id: combinedChatView
            visible: uiSettings.data.combinedChatWindow || false
            Layout.fillWidth: true
            Layout.fillHeight: true

            property QtObject currentContact: (visible && width > 0) ? contactList.selectedContact : null
            onCurrentContactChanged: {
                if (currentContact !== null) {
                    show(currentContact.uniqueID, Qt.resolvedUrl("ChatPage.qml"),
                         { 'contact': currentContact });
                } else {
                    currentKey = ""
                }
            }
        }
    }

    property bool inactive: true
    onActiveFocusItemChanged: {
        // Focus current page when window regains focus
        if (activeFocusItem !== null && inactive) {
            inactive = false
            retakeFocus.start()
        } else if (activeFocusItem === null) {
            inactive = true
        }
    }

    Timer {
        id: retakeFocus
        interval: 1
        onTriggered: {
            if (combinedChatView.currentPage !== null)
                combinedChatView.currentPage.forceActiveFocus()
        }
    }
}

