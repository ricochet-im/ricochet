import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import "ContactWindow.js" as ContactWindow

ApplicationWindow {
    id: msgNotification
    width: getWidth()
    height: getHeight()
    color: "#2d80c1"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.X11BypassWindowManagerHint | Qt.Popup

    x: (uiSettings.data.movieStyleNotification || false) ? (Screen.width - width) / 2 : Screen.desktopAvailableWidth - width - 10
    y: (uiSettings.data.movieStyleNotification || false) ? (Screen.height - height) / 2 : Screen.desktopAvailableHeight - height - 10

    function getWidth()
    {
        if (uiSettings.data.movieStyleNotification || false)
            return msgText.width + 80
        else
            return msgText.width + 20
    }

    function getHeight()
    {
        if (uiSettings.data.movieStyleNotification || false)
            return msgText.height + 60
        else
            return msgText.height + 20
    }

    signal closed

    function show()
    {
        visible = true
        notificationTimer.restart()
    }

    function hide()
    {
        visible = false
        notificationTimer.stop()
    }

    Rectangle {
        color: "#fff"
        width: parent.width - 4
        height: parent.height - 4
        anchors.centerIn: parent

        Text {
            id: msgText
            text: qsTr("You have a new message!")
            font.bold: true
            font.pixelSize: 18
            anchors.centerIn: parent
            color: "#2d80c1"
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: {
                // left button to show mainWindow
                if (mouse.button == Qt.LeftButton)
                    root.mainWindow.show()

                // other acceptedButtons only hide notification
                hide()
            }
        }
    }

    Timer {
        id: notificationTimer
        interval: uiSettings.read("notificationDuration", 3.0) * 1000
        repeat: false
        running: true
        onTriggered: { msgNotification.visible = false; }
    }
}

