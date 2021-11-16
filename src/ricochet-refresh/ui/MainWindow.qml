import QtQuick 2.2
import QtQuick.Window 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0
import "ContactWindow.js" as ContactWindow

ApplicationWindow {
    id: window
    title: "Ricochet"
    visibility: Window.AutomaticVisibility

    width: 650
    height: 400
    minimumHeight: 400
    minimumWidth: uiSettings.data.combinedChatWindow ? 650 : 250

    onMinimumWidthChanged: width = Math.max(width, minimumWidth)

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
        function onUnreadCountChanged(user, unreadCount) {
            if (unreadCount > 0) {
                if (audioNotifications !== null)
                    audioNotifications.message.play()
                var w = window
                if (!uiSettings.data.combinedChatWindow || ContactWindow.windowExists(user))
                    w = ContactWindow.getWindow(user)
                // On OS X, avoid bouncing the dock icon forever
                w.alert(Qt.platform.os == "osx" ? 1000 : 0)
            }
        }
        function onContactStatusChanged(user, status) {
            if (status === ContactUser.Online && audioNotifications !== null) {
                audioNotifications.contactOnline.play()
            }
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

                Accessible.role: Accessible.ToolBar
                //: Name of the main toolbar for accessibility tech like screen readers
                Accessible.name: qsTr("Main Toolbar")
                //: Description of the main toolbar for accessibility tech like screen readers
                Accessible.description: qsTr("Toolbar with connection status, add contact button, and preferences button")
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                ContactList {
                    id: contactList
                    anchors.fill: parent
                    opacity: offlineLoader.item !== null ? (1 - offlineLoader.item.opacity) : 1

                    function onContactActivated(contact, actions) {
                        if (contact.status === ContactUser.RequestPending || contact.status === ContactUser.RequestRejected) {
                            actions.openPreferences()
                        } else if (!uiSettings.data.combinedChatWindow) {
                            actions.openWindow()
                        }
                    }

                    Accessible.role: Accessible.Pane
                    //: Name of the pane holding the user's contacts for accessibility tech like screen readers
                    Accessible.name: qsTr("Contact pane")
                }

                Loader {
                    id: offlineLoader
                    active: torControl.torStatus !== TorControl.TorReady
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

                    // remove chat page for user when they are deleted
                    if(typeof currentContact.contactDeletedCallbackAdded === 'undefined') {
                        currentContact.contactDeleted.connect(function(user) {
                            remove(user.contactID);
                        });
                        currentContact.contactDeletedCallbackAdded = true;
                    }
                    show(currentContact.contactID, Qt.resolvedUrl("ChatPage.qml"),
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

    Action {
        shortcut: StandardKey.Close
        onTriggered: window.close()
    }
}

