import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0
import "utils.js" as Utils

Item {
    anchors.fill: parent

    property var bootstrap: torInstance.control.bootstrapStatus

    Column {
        id: info
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 8
        }
        spacing: 6

        GridLayout {
            columns: 4
            width: parent.width
            Label { text: qsTr("Running:") }
            Label { font.bold: true; Layout.fillWidth: true; text: (torInstance.process ? (torInstance.process.state == TorProcess.Ready ? qsTr("Yes") : qsTr("No")) : qsTr("External")) }
            Label { text: qsTr("Control connected:") }
            Label { font.bold: true; Layout.fillWidth: true; text: ((torInstance.control.status == TorControl.Connected) ? qsTr("Yes") : qsTr("No")) }
            Label { text: qsTr("Circuits established:") }
            Label { font.bold: true; text: ((torInstance.control.torStatus == TorControl.TorReady) ? qsTr("Yes") : qsTr("No")) }
            Label { text: qsTr("Hidden service:") }
            Label { font.bold: true; text: (userIdentity.isOnline ? qsTr("Online") : qsTr("Offline")) }
            Label { text: qsTr("Version:") }
            Label { font.bold: true; text: torControl.torVersion; textFormat: Text.PlainText }
            //Label { text: "Recommended:" }
            //Label { font.bold: true; text: "Unknown" }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: palette.mid
        }

        Label {
            text: bootstrap.summary
            visible: bootstrap.tag !== 'done'
        }

        ProgressBar {
            width: parent.width
            maximumValue: 100
            indeterminate: bootstrap.progress === undefined
            value: bootstrap.progress === undefined ? 0 : bootstrap.progress
            visible: bootstrap.tag !== 'done'
        }

        Label {
            //: %1 is error message
            text: qsTr("Error: <b>%1</b>").arg(Utils.htmlEscaped(errorMessage))
            visible: errorMessage != ""

            property string errorMessage: {
                if (torInstance.hasError)
                    return torInstance.errorMessage
                else if (torInstance.control.errorMessage != "")
                    return torInstance.control.errorMessage
                else if (bootstrap.warning !== undefined)
                    return bootstrap.warning
                else
                    return ""
            }
        }

        Button {
            text: qsTr("Configure")
            visible: torControl.hasOwnership
            onClicked: {
                var object = createDialog("NetworkSetupWizard.qml")
                object.visible = true
            }
        }
    }

    TorLogDisplay {
        anchors {
            left: parent.left
            right: parent.right
            top: info.bottom
            bottom: parent.bottom
            margins: 8
        }
        visible: torInstance.process !== null
    }
}

