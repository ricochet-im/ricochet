import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0


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
            Label { text: "Running:" }
            Label { font.bold: true; Layout.fillWidth: true; text: (torInstance.process ? (torInstance.process.state == TorProcess.Ready ? "Yes" : "No") : "External") }
            Label { text: "Control connected:" }
            Label { font.bold: true; Layout.fillWidth: true; text: ((torInstance.control.status == TorControl.Connected) ? "Yes" : "No") }
            Label { text: "Circuits established:" }
            Label { font.bold: true; text: ((torInstance.control.torStatus == TorControl.TorReady) ? "Yes" : "No") }
            Label { text: "Hidden service:" }
            Label { font.bold: true; text: (userIdentity.isServiceOnline ? "Online" : (userIdentity.isServicePublished ? "Published" : "Offline")) }
            Label { text: "Version:" }
            Label { font.bold: true; text: torControl.torVersion }
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
            text: "Error: <b>" + errorMessage + "</b>"
            visible: errorMessage != ""

            property string errorMessage: {
                if (torInstance.process !== null && torInstance.process.errorMessage != "")
                    return torInstance.process.errorMessage
                else if (torInstance.control.errorMessage != "")
                    return torInstance.control.errorMessage
                else if (bootstrap.warning !== undefined)
                    return bootstrap.warning
                else
                    return ""
            }
        }

        Button {
            text: "Configure"
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

