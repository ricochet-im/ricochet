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
    property ContactUser contact

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
            //: Label displayed when user clicks on a link
            text: qsTr("<b>Warning!</b> Opening links with your default browser will harm your security and anonymity.<br><br>You can <a href='.'>copy to the clipboard</a> instead.")
            wrapMode: Text.Wrap
            horizontalAlignment: Qt.AlignHCenter
            onLinkActivated: {
                LinkedText.copyToClipboard(dialog.link)
                dialog.close()
            }
            Accessible.role: Accessible.StaticText
            //: Name of warning label used with accessibility tech such as screen readers
            Accessible.name: qsTr("Warning")
            Accessible.description: text
        }

        Item { width: 1; height: 1 }

        Rectangle {
            height: 1
            Layout.fillWidth: true
            color: Qt.darker(palette.window, 1.5)
        }

        CheckBox {
            id: alwaysOpenAll
            //: Checkbox option text for when user clicks on a link
            text: qsTr("Don't ask again for any links (not recommended!)")
            checked: uiSettings.data.alwaysOpenBrowser || false
        }

        RowLayout {
            width: parent.width
            Button {
                //: Label on button to open link in a web browser
                text: qsTr("Open Browser")
                onClicked: {
                    if (alwaysOpenAll.checked)
                        uiSettings.write("alwaysOpenBrowser", true)
                    Qt.openUrlExternally(link)
                    dialog.close()
                }
            }
            Item { Layout.fillWidth: true; height: 1 }
            Button {
                //: Label on cancel button
                text: qsTr("Cancel")
                isDefault: true
                onClicked: dialog.close()
            }
        }

        Item { height: Qt.platform.os === "linux" ? 8 : 0}

        Keys.onEscapePressed: dialog.close()
        Keys.onReturnPressed: dialog.close()
    }
}
