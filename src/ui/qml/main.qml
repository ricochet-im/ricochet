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
            var object = createDialog("AddContactDialog.qml")
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

    function createDialog(component, properties) {
        if (typeof(component) === "string")
            component = Qt.createComponent(component)
        if (component.status !== Component.Ready)
            console.log("openDialog:", component.errorString())
        var object = component.createObject(window, (properties !== undefined) ? properties : { })
        if (!object)
            console.log("openDialog:", component.errorString())
        object.visibleChanged.connect(function() { if (!object.visible) object.destroy() })
        return object
    }

    function openPreferences(page, properties) {
        var object = createDialog("PreferencesDialog.qml",
            {
                'initialPage': page,
                'initialPageProperties': properties
            }
        )
        object.visible = true
    }

    SystemPalette {
        id: palette
    }

    Connections {
        target: userIdentity.contacts.incomingRequests
        onRequestAdded: {
            var object = createDialog("ContactRequestDialog.qml", { 'request': request })
            object.visible = true
        }
    }

    Connections {
        target: userIdentity.contacts
        onPrepareInteractiveHandler: {
            ContactWindow.getWindow(user)
        }
    }

    Connections {
        target: torInstance
        onConfigurationNeededChanged: {
            if (torInstance.configurationNeeded) {
                var object = createDialog("NetworkSetupWizard.qml", { 'modality': Qt.ApplicationModal })
                object.networkReady.connect(function() { object.visible = false })
                object.visible = true
            }
        }
    }

    Component.onCompleted: {
        ContactWindow.createWindow = function(user) {
            var re = createDialog("ChatWindow.qml", { 'contact': user })
            re.x = window.x + window.width + 10
            re.y = window.y + (window.height / 2) - (re.height / 2)
            re.visible = true
            return re
        }

        if (torInstance.configurationNeeded) {
            var object = createDialog("NetworkSetupWizard.qml")
            object.networkReady.connect(function() {
                window.visible = true
                object.visible = false
            })
            object.visible = true
        } else {
            window.visible = true
        }
    }

    Timer {
        interval: 2000
        running: true
        repeat: false
        onTriggered: {
            var pendingRequests = userIdentity.contacts.incomingRequests.requests
            for (var i = 0; i < pendingRequests.length; i++) {
                var object = createDialog("ContactRequestDialog.qml", { 'request': pendingRequests[i] })
                object.visible = true
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
