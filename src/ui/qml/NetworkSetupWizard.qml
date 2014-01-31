import QtQuick 2.0
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
    title: qsTr("Torsion")

    property Item visibleItem: (configLoader.visible && configLoader.active) ? configLoader.item : pageLoader.item

    function back() {
        if (pageLoader.visible && configLoader.active) {
            pageLoader.visible = false
            configLoader.visible = true
        } else {
            openBeginning()
        }
    }

    function openBeginning() {
        configLoader.visible = false
        configLoader.active = false
        pageLoader.sourceComponent = firstPage
        pageLoader.visible = true
    }

    function openConfig() {
        configLoader.active = true
        pageLoader.visible = false
        configLoader.visible = true
    }

    function openBootstrap() {
        configLoader.visible = false
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

    Loader {
        id: configLoader
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 8
        }
        source: Qt.resolvedUrl("TorConfigurationPage.qml")
        active: false
    }

    Component {
        id: firstPage

        Column {
            spacing: 8

            Label {
                width: parent.width
                text: "This computer's Internet connection is free of obstacles. " +
                      "I would like to connect directly to the Tor network."
                wrapMode: Text.Wrap
                horizontalAlignment: Qt.AlignHCenter
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Connect"
                isDefault: true
                onClicked: window.openBootstrap()
            }

            Rectangle {
                height: 1
                width: parent.width
                color: palette.mid
            }

            Label {
                width: parent.width
                text: "This computer's Internet connection is censored, filtered, or proxied. " +
                      "I need to configure network settings."
                wrapMode: Text.Wrap
                horizontalAlignment: Qt.AlignHCenter
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Configure"
                onClicked: window.openConfig()
            }
        }
    }

    Behavior on minimumHeight {
        enabled: pageLoader.status === Loader.Ready
        SmoothedAnimation {
            easing.type: Easing.InOutQuad
            velocity: 1500
        }
    }
}
