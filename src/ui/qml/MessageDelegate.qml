import QtQuick 2.0
import QtQuick.Controls 1.0
import im.ricochet 1.0

Column {
    id: delegate
    width: parent.width

    Loader {
        active: model.section === "offline"
        sourceComponent: Label {
            //: %1 nickname
            text: qsTr("%1 is offline").arg(contact !== null ? contact.nickname : "")
            width: background.parent.width
            elide: Text.ElideRight
            horizontalAlignment: Qt.AlignHCenter
            color: palette.mid

            Rectangle {
                id: line
                width: (parent.width - parent.contentWidth) / 2 - 4
                height: 1
                y: (parent.height - 1) / 2
                color: Qt.lighter(palette.mid, 1.4)
            }

            Rectangle {
                width: line.width
                height: 1
                y: line.y
                x: parent.width - width
                color: line.color
            }
        }
    }

    Rectangle {
        id: background
        width: Math.max(30, textField.width + 12)
        height: textField.height + 12
        x: model.isOutgoing ? parent.width - width - 10 : 10

        property int __maxWidth: parent.width * 0.8

        color: (model.status === ConversationModel.Error) ? "#ffdcc4" : ( model.isOutgoing ? "#eaeced" : "#c4e7ff" )
        Behavior on color { ColorAnimation { } }

        Rectangle {
            rotation: 45
            width: 10
            height: 10
            x: model.isOutgoing ? parent.width - 20 : 10
            y: model.isOutgoing ? parent.height - 5 : -5
            color: parent.color
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            opacity: (model.status === ConversationModel.Sending || model.status === ConversationModel.Queued || model.status === ConversationModel.Error) ? 1 : 0
            visible: opacity > 0
            color: Qt.lighter(parent.color, 1.15)

            Behavior on opacity { NumberAnimation { } }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton

            onClicked: delegate.showContextMenu()
        }

        TextEdit {
            id: textField
            width: Math.min(implicitWidth, background.__maxWidth)
            height: contentHeight
            x: Math.round((parent.width - width) / 2)
            y: 6

            renderType: Text.NativeRendering
            textFormat: TextEdit.RichText
            selectionColor: palette.highlight
            selectedTextColor: palette.highlightedText
            font.pointSize: styleHelper.pointSize

            wrapMode: TextEdit.Wrap
            readOnly: true
            selectByMouse: true
            text: LinkedText.parsed(model.text)

            onLinkActivated: delegate.showContextMenu(link)

            // Workaround an incomplete fix for QTBUG-31646
            Component.onCompleted: {
                if (textField.hasOwnProperty('linkHovered'))
                    textField.linkHovered.connect(function() { })
            }
        }
    }

    function showContextMenu(link) {
        var object = contextMenu.createObject(delegate, (link !== undefined) ? { 'hoveredLink': link } : { })
        object.visibleChanged.connect(function() { if (!object.visible) object.destroy() })
        object.popup()
    }

    Component {
        id: contextMenu

        Menu {
            property string hoveredLink: textField.hasOwnProperty('hoveredLink') ? textField.hoveredLink : ""
            MenuItem {
                text: linkAddContact.visible ? qsTr("Copy ID") : qsTr("Copy Link")
                visible: hoveredLink.length > 0
                onTriggered: LinkedText.copyToClipboard(hoveredLink)
            }
            MenuItem {
                text: qsTr("Open with Browser")
                visible: hoveredLink.length > 0 && hoveredLink.substr(0,4).toLowerCase() == "http"
                onTriggered: {
                    if (uiSettings.data.alwaysOpenBrowser || contact.settings.data.alwaysOpenBrowser) {
                        Qt.openUrlExternally(hoveredLink)
                    } else {
                        var window = uiMain.findParentWindow(delegate)
                        var object = createDialog("OpenBrowserDialog.qml", { 'link': hoveredLink, 'contact': contact }, window)
                        object.visible = true
                    }
                }
            }
            MenuItem {
                id: linkAddContact
                text: qsTr("Add as Contact")
                visible: hoveredLink.length > 0 && (hoveredLink.substr(0,9).toLowerCase() == "ricochet:"
                                                    || hoveredLink.substr(0,8).toLowerCase() == "torsion:")
                onTriggered: {
                    var object = createDialog("AddContactDialog.qml", { 'staticContactId': hoveredLink }, chatWindow)
                    object.visible = true
                }
            }
            MenuSeparator {
                visible: hoveredLink.length > 0
            }
            MenuItem {
                text: qsTr("Copy")
                shortcut: "Ctrl+C"
                onTriggered: {
                    if (textField.selectedText.length > 0) {
                        textField.copy()
                    } else {
                        textField.selectAll()
                        textField.copy()
                        textField.deselect()
                    }
                }
            }
        }
    }
}
