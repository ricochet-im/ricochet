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
        text: "Add Contact"
        onTriggered: {
            var object = createDialog("AddContactDialog.qml", { }, window)
            object.visible = true
        }
    }

    Action {
        id: preferencesAction
        iconSource: "qrc:/ui/icons/gear.png"
        text: "Preferences"
        onTriggered: {
            var object = createDialog("PreferencesDialog.qml")
            object.visible = true
        }
    }

    toolBar: ToolBar {
        RowLayout {
            id: toolBarLayout
            width: parent.width

            TorStateWidget { }

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
                    sourceComponent: EmptyContactsHint {
                        targetButton: addContactButton
                        maximumWidth: toolBarLayout.width
                    }
                }
            }

            ToolButton {
                action: preferencesAction
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
