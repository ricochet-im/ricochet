import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ColumnLayout {
    anchors {
        fill: parent
        margins: 8
    }

    Label {
        Layout.fillWidth: true
        //: %1 version, e.g. 1.0.0
        text: qsTr("Ricochet %1").arg(uiMain.version)
        horizontalAlignment: Qt.AlignHCenter
    }

    Label {
        Layout.fillWidth: true
        text: "<a href='https://ricochetrefresh.net/'>ricochetrefresh.net</a>"
        horizontalAlignment: Qt.AlignHCenter

        MouseArea {
            anchors.fill: parent
            onClicked: Qt.openUrlExternally("https://ricochetrefresh.net/")
        }
    }

    TextArea {
        Layout.fillHeight: true
        Layout.fillWidth: true
        readOnly: true
        text: uiMain.aboutText
    }
}

