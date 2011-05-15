import Qt 4.7

Image {
    fillMode: Image.PreserveAspectFit
    smooth: true

    BorderImage {
        source: "avatar-border.png"
        border.bottom: 2
        border.top: 2
        border.left: 2
        border.right: 2
        width: parent.paintedWidth+2
        height: parent.paintedHeight+2
        x: -1
        y: -1
    }
}
