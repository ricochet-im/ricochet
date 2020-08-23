import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: window
    width: minimumWidth
    height: minimumHeight
    minimumWidth: 400
    maximumWidth: minimumWidth
    minimumHeight: visibleItem.height + 16
    maximumHeight: minimumHeight
    title: "Ricochet"

    signal networkReady
    signal closed

    onVisibleChanged: if (!visible) closed()

    property Item visibleItem: configPage.visible ? configPage : pageLoader.item

    function back() {
        if (pageLoader.visible) {
            pageLoader.visible = false
            configPage.visible = true
        } else {
            openBeginning()
        }
    }

    function openBeginning() {
        configPage.visible = false
        configPage.reset()
        pageLoader.sourceComponent = firstPage
        pageLoader.visible = true
    }

    function openConfig() {
        pageLoader.visible = false
        configPage.visible = true
    }

    function openBootstrap() {
        configPage.visible = false
        pageLoader.source = Qt.resolvedUrl("TorBootstrapStatus.qml")
        pageLoader.visible = true
    }

    Loader {
        id: pageLoader
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 8
        }
        sourceComponent: firstPage
    }

    TorConfigurationPage {
        id: configPage
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 8
        }
        visible: false
    }

    StartupStatusPage {
        id: statusPage
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 8
        }
        visible: false

        onHasErrorChanged: {
            if (hasError) {
                if (visibleItem)
                    visibleItem.visible = false
                pageLoader.visible = false
                statusPage.visible = true
                visibleItem = statusPage
            }
        }
    }

    Component {
        id: firstPage

        Column {
            spacing: 8

            Label {
                width: parent.width
                text: qsTr("This computer's Internet connection is free of obstacles. I would like to connect directly to the Tor network.")
                wrapMode: Text.Wrap
                horizontalAlignment: Qt.AlignHCenter
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Connect")
                isDefault: true
                onClicked: {
                    // Reset to defaults and proceed to bootstrap page
                    configPage.reset()
                    configPage.save()
                }
            }

            Rectangle {
                height: 1
                width: parent.width
                color: palette.mid
            }

            Label {
                width: parent.width
                text: qsTr("This computer's Internet connection is censored, filtered, or proxied. I need to configure network settings.")
                wrapMode: Text.Wrap
                horizontalAlignment: Qt.AlignHCenter
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Configure")
                onClicked: window.openConfig()
            }
        }
    }

    Action {
        shortcut: StandardKey.Close
        onTriggered: window.close()
    }
}
