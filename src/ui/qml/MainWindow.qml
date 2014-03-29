import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0
import "ContactWindow.js" as ContactWindow

ApplicationWindow {
    id: window
    width: 250
    height: 400
    maximumWidth: width
    maximumHeight: height
    minimumWidth: width
    minimumHeight: height
    title: qsTr("Torsion")
    visibility: Window.AutomaticVisibility

    Action {
        id: addContactAction
        // CC-BY, Plus by Andre from The Noun Project
        iconSource: "qrc:/ui/icons/plus.png"
        text: qsTr("Add Contact")
        onTriggered: {
            var object = createDialog("AddContactDialog.qml", { }, window)
            object.visible = true
        }
    }

    Action {
        id: preferencesAction
        text: qsTr("Preferences")
        onTriggered: {
            var object = createDialog("PreferencesDialog.qml")
            object.visible = true
        }
    }

    // OS X Menu
    Loader {
        active: Qt.platform.os == 'osx'
        sourceComponent: MenuBar {
            Menu {
                title: qsTr("Torsion")
                MenuItem {
                    text: qsTranslate("QCocoaMenuItem", "Preference")
                    onTriggered: preferencesAction.trigger()
                }
            }
        }
    }

    toolBar: ToolBar {
        RowLayout {
            id: toolBarLayout
            width: parent.width

            TorStateWidget {
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                Layout.fillWidth: true
                height: 1
            }

            ToolButton {
                id: addContactButton
                action: addContactAction
                implicitHeight: 24
                implicitWidth: 24

                Loader {
                    id: emptyState
                    active: contactList.view.count == 0
                    sourceComponent: Bubble {
                        target: addContactButton
                        maximumWidth: toolBarLayout.width
                        text: qsTr("Click to add contacts")
                    }
                }
            }

            ToolButton {
                action: preferencesAction
                iconSource: "qrc:/ui/icons/gear.png"
                implicitHeight: 24
                implicitWidth: 24
            }
        }
    }

    ContactList {
        id: contactList
        anchors.fill: parent
        opacity: offlineLoader.item !== null ? (1 - offlineLoader.item.opacity) : 1

        onContactActivated: {
            if (contact.status === ContactUser.RequestPending || contact.status === ContactUser.RequestRejected) {
                actions.openPreferences()
            } else {
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
