import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

ApplicationWindow {
    id: dialog
    width: 400
    height: layout.height + 24
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height
    flags: Qt.Dialog
    modality: Qt.WindowModal

    signal closed
    onVisibleChanged: if (!visible) closed()

    function close() { visible = false }

    property string link

    ColumnLayout {
        id: layout
        focus: true
        spacing: 8
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 8
            topMargin: 16
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("<b>Warning!</b> Using your default browser may compromise your anonymity. Are you sure you want to open this link:")
            wrapMode: Text.Wrap
            horizontalAlignment: Qt.AlignHCenter
        }

        Label {
            Layout.fillWidth: true
            text: dialog.link
            textFormat: Text.PlainText
            horizontalAlignment: Qt.AlignHCenter
            elide: Text.ElideMiddle
        }

        RowLayout {
            width: parent.width
            Button {
                text: qsTr("Open Browser")
                onClicked: {
                    Qt.openUrlExternally(link)
                    dialog.close()
                }
            }
            Item { Layout.fillWidth: true; height: 1 }
            Button {
                text: qsTr("Copy Link")
                onClicked: {
                    LinkedText.copyToClipboard(link)
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
