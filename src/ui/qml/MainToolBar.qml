import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtQuick.Controls.Styles 1.0
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
            text: qsTr("Add Contact")
            onTriggered: {
                var object = createDialog("AddContactDialog.qml", { }, window)
                object.visible = true
            }
        },

        Action {
            id: preferencesAction
            text: qsTr("Preferences")
            onTriggered: root.openPreferences()
        }
    ]

    Component {
        id: iconButtonStyle

        ButtonStyle {
            background: Item { }
            label: Text {
                text: control.text
                font.family: iconFont.name
                font.pixelSize: height
                horizontalAlignment: Qt.AlignHCenter
                renderType: Text.QtRendering
                color: "black"
            }
        }
    }

    RowLayout {
        id: toolBarLayout
        width: parent.width

        TorStateWidget {
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            Layout.fillWidth: true
            height: 1
        }

        ToolButton {
            id: addContactButton
            implicitHeight: 24
            action: addContactAction
            style: iconButtonStyle
            text: "\ue810" // iconFont plus symbol

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
            implicitHeight: 24
            style: iconButtonStyle
            text: "\ue803" // iconFont gear
        }
    }
}
