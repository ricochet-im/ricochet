import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15

ColumnLayout {
    anchors {
        fill: parent
        margins: 8
    }

    Label {
        Layout.fillWidth: true
        //: %1 version, e.g. 1.0.0
        text: qsTr("Ricochet-Refresh %1").arg(uiMain.version)
        horizontalAlignment: Qt.AlignHCenter

        Accessible.description: qsTr("Current Ricochet-Refresh version")
        Accessible.role: Accessible.StaticText
        //: provides a readable interpretation of the Ricochet-Refresh version number for accessibility tech like screen readers
        Accessible.name: qsTr("Ricochet Refresh version %1").arg(uiMain.accessibleVersion)
    }

    Label {
        horizontalAlignment: Qt.AlignHCenter
        Layout.fillWidth: true

        Label {
            id: homePageLink
            anchors.centerIn: parent

            text: "<a href='https://ricochetrefresh.net/'>ricochetrefresh.net</a>"
            onLinkActivated: Qt.openUrlExternall("https://ricochetrefresh.net")
        }

        //: provides context for the URL for accessibility tech like screen readers
        Accessible.name: qsTr("Ricochet Refresh web home page")
        Accessible.role: Accessible.StaticText
    }

    Flickable {
        // this is so that the scroll bars doesn't spill over the border
        Layout.rightMargin: 8
        Layout.bottomMargin: 8
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        TextArea.flickable : TextArea {
            selectByMouse: true

            readOnly: true
            text: uiMain.aboutText
        }

        ScrollBar.vertical: ScrollBar { }
        ScrollBar.horizontal: ScrollBar { } // openssl license has some long lines that need to be accounted for

        //: summary of a block of text for accessibility tech like screen readers
        Accessible.description: qsTr("The license of Ricochet Refresh and its dependencies")
        Accessible.name: qsTr("License")
        Accessible.role: Accessible.StaticText
    }

    Accessible.role: Accessible.Window
    Accessible.name: qsTr("About")
    //: summary of the window's contents for accessibility tech like screen readers
    Accessible.description: qsTr("About page, contains license and version information")
}
