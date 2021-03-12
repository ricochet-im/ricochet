import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

MouseArea {
    id: offlineState
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    visible: opacity > 0
    enabled: visible
    opacity: 0
    clip: true

    Behavior on opacity { NumberAnimation { duration: 500 } }

    Rectangle {
        anchors.fill: parent
        color: palette.base
    }

    Label {
        id: label
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: parent.height / -3
        }
        font.pointSize: 14
    }

    Rectangle {
        id: indicator
        width: label.width
        anchors {
            top: label.bottom
            topMargin: 2
        }
        height: 2
        x: label.x

        onWidthChanged: if (indicatorAnimation.running) indicatorAnimation.restart()

        property alias running: indicatorAnimation.running

        SequentialAnimation {
            id: indicatorAnimation

            function restart() {
                stop()
                animation1.to = offlineState.width
                animation2.from = -indicator.width
                animation2.to = offlineState.width
                start()
            }

            NumberAnimation {
                id: animation1
                target: indicator
                property: "x"
                to: offlineState.width
                duration: 500
                easing.type: Easing.InQuad
            }

            NumberAnimation {
                id: animation2
                loops: Animation.Infinite
                target: indicator
                property: "x"
                from: -indicator.width
                to: offlineState.width
                duration: 1500
                easing.type: Easing.OutInQuad
            }
        }
    }

    onWidthChanged: if (indicatorAnimation.running) indicatorAnimation.restart()

    Label {
        id: detailedLabel
        anchors {
            left: parent.left
            right: parent.right
            top: indicator.bottom
            margins: 16
        }
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        color: Qt.lighter(palette.text, 1.2)
        font.pointSize: 11
        text: torControl.errorMessage
    }

    GridLayout {
        id: buttonRow
        visible: false
        anchors {
            left: parent.left
            right: parent.right
            top: detailedLabel.bottom
            margins: 16
            topMargin: 32
        }
        Button {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            //: Button label
            text: qsTr("Configure")
            onClicked: {
                var object = createDialog("NetworkSetupWizard.qml", { }, window)
                object.visible = true
            }
        }
        Button {
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            //: Button label
            text: qsTr("Details")
            onClicked: {
                openPreferences("TorPreferences.qml")
            }
        }
    }

    states: [
        State {
            name: "connected"
            when: torControl.torStatus === TorControl.TorReady

            PropertyChanges {
                target: offlineState
                opacity: 0
            }
        },
        State {
            name: "failed"
            when: torControl.status === TorControl.Error

            PropertyChanges {
                target: offlineState
                opacity: 1
            }

            PropertyChanges {
                target: label
                //: Label displayed when connecting to the Tor network fails
                text: qsTr("Connection failed")
            }

            PropertyChanges {
                target: indicator
                color: "#ffdcc4"
                running: false
            }

            PropertyChanges {
                target: buttonRow
                visible: true
            }
        },
        State {
            name: "connecting"
            when: torControl.torStatus !== TorControl.TorReady

            PropertyChanges {
                target: offlineState
                opacity: 1
            }

            PropertyChanges {
                target: label
                //: Label displayed when in process of connecting, \u2026 is ellipsis
                text: qsTr("Connecting\u2026")
            }

            PropertyChanges {
                target: indicator
                color: "#c4e7ff"
                running: true
                x: label.x
            }
        }
    ]

    transitions: [
        Transition {
            to: "connecting"

            SequentialAnimation {
                PropertyAction {
                    target: label
                    property: "text"
                }

                PropertyAction {
                    target: indicator
                    property: "running"
                }

                ColorAnimation {
                    target: indicator
                    property: "color"
                    duration: 1000
                }
            }
        },
        Transition {
            to: "failed"

            SequentialAnimation {
                PropertyAction {
                    target: indicator
                    property: "running"
                }

                PropertyAction {
                    target: label
                    property: "text"
                }

                ParallelAnimation {
                    NumberAnimation {
                        target: indicator
                        property: "x"
                        duration: 1000
                        easing.type: Easing.OutQuad
                    }

                    ColorAnimation {
                        target: indicator
                        property: "color"
                        duration: 1000
                    }
                }
            }
        }
    ]
}

