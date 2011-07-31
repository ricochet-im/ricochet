import org.torsionim.torsion 1.0
import Qt 4.7

Text {
    id: contactId
    font.pixelSize: 11
    font.family: "Consolas, Menlo, Andale Mono, Bitstream Vera Sans Mono, Courier New"

    property color realColor

    SequentialAnimation {
        id: copiedAnimation

        ColorAnimation {
            target: contactId
            property: "color"
            to: "#ffffff"
            duration: 100
        }

        ColorAnimation {
            target: contactId
            property: "color"
            to: contactId.realColor
            duration: 200
        }
    }

    MouseArea {
        anchors.fill: parent
        anchors.margins: -1

        onDoubleClicked: {
            helper.clipboardText = contactId.text
            contactId.realColor = contactId.color
            copiedAnimation.start()
        }
    }
}
