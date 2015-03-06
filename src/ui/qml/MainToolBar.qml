import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

ToolBar {
    Layout.minimumWidth: 200
    Layout.fillWidth: true
    // Necessary to avoid oversized toolbars, e.g. OS X with Qt 5.4.1
    implicitHeight: toolBarLayout.height + __style.padding.top + __style.padding.bottom

    property Action addContact: addContactAction
    property Action preferences: preferencesAction

    data: [
        Action {
            id: addContactAction
            // CC-BY, Plus by Andre from The Noun Project
            iconSource: "qrc:/ui/icons/plus.png"
            text: qsTr("Add Contact")
            onTriggered: {
                var object = createDialog("AddContactDialog.qml", { }, window)
                object.visible = true
            }
        },

        Action {
            id: preferencesAction
            text: qsTr("Preferences")
            onTriggered: {
                var object = createDialog("PreferencesDialog.qml")
                object.visible = true
            }
        }
    ]

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
