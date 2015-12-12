import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0
import "utils.js" as Utils

ApplicationWindow {
    id: dialog
    width: 400
    height: layout.height + 32
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height
    flags: styleHelper.dialogWindowFlags
    modality: Qt.WindowModal
    title: mainWindow.title

    signal closed
    onVisibleChanged: if (!visible) closed()

    function close() { visible = false }

    property string link
    property QtObject contact

    ColumnLayout {
        id: layout
        focus: true
        spacing: 8
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 16
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("<b>Warning!</b> Opening links with your default browser will harm your security and anonymity.<br><br>You can <a href='.'>copy to the clipboard</a> instead.")
            wrapMode: Text.Wrap
            horizontalAlignment: Qt.AlignHCenter
            onLinkActivated: {
                LinkedText.copyToClipboard(dialog.link)
                dialog.close()
            }
        }

        Item { width: 1; height: 1 }

        Rectangle {
            height: 1
            Layout.fillWidth: true
            color: Qt.darker(palette.window, 1.5)
        }

        CheckBox {
            id: alwaysOpenContact
            text: qsTr("Don't ask again for links from %1").arg(contact ? Utils.htmlEscaped(contact.nickname) : "???")
            checked: contact.settings.data.alwaysOpenBrowser || false
        }

        CheckBox {
            id: alwaysOpenAll
            text: qsTr("Don't ask again for any links (not recommended!)")
            checked: uiSettings.data.alwaysOpenBrowser || false
        }

        RowLayout {
            width: parent.width
            Button {
                text: qsTr("Open Browser")
                onClicked: {
                    if (alwaysOpenContact.checked)
                        contact.settings.write("alwaysOpenBrowser", true)
                    if (alwaysOpenAll.checked)
                        uiSettings.write("alwaysOpenBrowser", true)
                    Qt.openUrlExternally(link)
                    dialog.close()
                }
            }
            Item { Layout.fillWidth: true; height: 1 }
            Button {
                text: qsTr("Cancel")
                isDefault: true
                onClicked: dialog.close()
            }
        }

        Keys.onEscapePressed: dialog.close()
        Keys.onReturnPressed: dialog.close()
    }
}
