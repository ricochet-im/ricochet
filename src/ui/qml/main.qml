import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
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
    visible: true

    menuBar: MenuBar {
        Menu { title: "File"; MenuItem { text: "???" } }
    }

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
            width: parent.width

            TorStateWidget { }

            Item {
                Layout.fillWidth: true
                height: 1
            }

            ToolButton {
                action: addContactAction
                implicitHeight: 24
                implicitWidth: 24
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

    Component.onCompleted: {
        ContactWindow.createWindow = function(user) {
            var re = createDialog("ChatWindow.qml", { 'contact': user })
            re.visible = true
            return re
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
    }
}
