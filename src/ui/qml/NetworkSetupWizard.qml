import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: window
    width: minimumWidth
    height: minimumHeight
    minimumWidth: 600
    maximumWidth: minimumWidth
    minimumHeight: visibleItem.height + 32
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
            width: parent.width
            spacing: 20
            anchors {
                left: parent.left
                right: parent.right
                margins: parent.width/10
            }


            Column {
                width: parent.width
                spacing: 10

                Image {
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: "../icons/ricochet.svg"
                    sourceSize.width: parent.width / 10
                    sourceSize.height: 60
                }

                Label {
                    width: parent.width
                    horizontalAlignment: Qt.AlignHCenter
                    text: qsTr("Welcome to Ricochet. Before we start, we need to know about your connection to the Internet.")
                    wrapMode: Text.Wrap
                }
                
                Label {
                    width: parent.width
                    horizontalAlignment: Qt.AlignHCenter
                    text: qsTr("Please try <b>Connect</b> first if you do not require bridges or special network settings.")
                    wrapMode: Text.Wrap
                }
            }

            Row {
                width: parent.width
                spacing: 10

                Column {
                    width: parent.width/2.5
                    spacing: 10

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "../icons/connection_ok.svg"
                        sourceSize.width: parent.width
                        sourceSize.height: 50
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

                    Label {
                        width: parent.width
                        text: qsTr("This computer can freely access the Internet")
                        wrapMode: Text.Wrap
                        horizontalAlignment: Qt.AlignHCenter
                    }
                }

                Column {
                    width: parent.width/5

                    // We need something in this column such that the
                    // width will be rendered.
                    Label {
                        width: parent.width
                    }
                }


                Column {
                    width: parent.width/2.5
                    spacing: 10

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "../icons/connection_censored.svg"
                        sourceSize.width: parent.width
                        sourceSize.height: 50
                    }

                    Button {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Configure")
                        onClicked: window.openConfig()
                    }

                    Label {
                        width: parent.width
                        text: qsTr("My connection is censored, filtered, or requires a proxy.")
                        wrapMode: Text.Wrap
                        horizontalAlignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }

    Behavior on height {
        // This window animation causes bad graphical behavior on Windows with 5.4.1
        enabled: Qt.platform.os !== "windows"
        SmoothedAnimation {
            easing.type: Easing.InOutQuad
            velocity: 1500
        }
    }
}
