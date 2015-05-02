import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.0
import im.ricochet 1.0
import "ContactWindow.js" as ContactWindow

// Root non-graphical object providing window management and other logic.
QtObject {
    id: root

    property MainWindow mainWindow: MainWindow {
        onVisibleChanged: if (!visible) Qt.quit()
    }

    function createDialog(component, properties, parent) {
        if (typeof(component) === "string")
            component = Qt.createComponent(component)
        if (component.status !== Component.Ready)
            console.log("openDialog:", component.errorString())
        var object = component.createObject(parent ? parent : null, (properties !== undefined) ? properties : { })
        if (!object)
            console.log("openDialog:", component.errorString())
        object.closed.connect(function() { object.destroy() })
        return object
    }

    property QtObject preferencesDialog
    function openPreferences(page, properties) {
        if (preferencesDialog == null) {
            preferencesDialog = createDialog("PreferencesDialog.qml",
                {
                    'initialPage': page,
                    'initialPageProperties': properties
                }
            )
            preferencesDialog.closed.connect(function() { preferencesDialog = null })
        }

        preferencesDialog.visible = true
        preferencesDialog.raise()
        preferencesDialog.requestActivate()
    }

    Component.onCompleted: {
        ContactWindow.createWindow = function(user) {
            var re = createDialog("ChatWindow.qml", { 'contact': user })
            re.x = mainWindow.x + mainWindow.width + 10
            re.y = mainWindow.y + (mainWindow.height / 2) - (re.height / 2)

            var screens = uiMain.screens
            if ((mainWindow.Screen !== undefined) && (mainWindow.Screen.name in screens)) {
                var currentScreen = screens[mainWindow.Screen.name]
                var offsetX = currentScreen.left
                var offsetY = currentScreen.top
                re.x = re.x - offsetX + re.width <= currentScreen.width ? re.x : mainWindow.x - re.width - 10
                re.y = re.y - offsetY + re.height <= currentScreen.height ? re.y : currentScreen.height + offsetY - re.height - 10
            }

            re.visible = true
            return re
        }

        if (torInstance.configurationNeeded) {
            var object = createDialog("NetworkSetupWizard.qml")
            object.networkReady.connect(function() {
                mainWindow.visible = true
                object.visible = false
            })
            object.visible = true
        } else {
            mainWindow.visible = true
        }
    }

    property list<QtObject> data: [
        Connections {
            target: userIdentity.contacts.incomingRequests
            onRequestAdded: {
                var object = createDialog("ContactRequestDialog.qml", { 'request': request }, mainWindow)
                object.visible = true
            }
        },

        Connections {
            target: torInstance
            onConfigurationNeededChanged: {
                if (torInstance.configurationNeeded) {
                    var object = createDialog("NetworkSetupWizard.qml", { 'modality': Qt.ApplicationModal }, mainWindow)
                    object.networkReady.connect(function() { object.visible = false })
                    object.visible = true
                }
            }
        },

        Settings {
            id: uiSettings
            path: "ui"
        },

        SystemPalette {
            id: palette
        },

        FontLoader {
            id: iconFont
            source: "qrc:/icons/ricochet_icons.ttf"
        },

        Item {
            id: styleHelper
            visible: false
            Label { id: fakeLabel }
            Label { id: fakeLabelSized; font.pointSize: styleHelper.pointSize > 0 ? styleHelper.pointSize : 1 }

            property int pointSize: (Qt.platform.os === "windows") ? 10 : fakeLabel.font.pointSize
            property int textHeight: fakeLabelSized.height
            property int dialogWindowFlags: Qt.Dialog | Qt.WindowSystemMenuHint | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
        },

        Timer {
            interval: 2000
            running: true
            repeat: false
            onTriggered: {
                var pendingRequests = userIdentity.contacts.incomingRequests.requests
                for (var i = 0; i < pendingRequests.length; i++) {
                    var object = createDialog("ContactRequestDialog.qml", { 'request': pendingRequests[i] }, mainWindow)
                    object.visible = true
                }
            }
        },

        Binding {
            target: audioNotification
            property: "enabled"
            value: uiSettings.data.playAudioNotification
        },

        Binding {
            target: audioNotification
            property: "volume"
            value: uiSettings.data.notificationVolume
        }
    ]
}
