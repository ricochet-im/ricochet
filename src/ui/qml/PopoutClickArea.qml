import Qt 4.7

MouseArea {
    property PopoutItem target

    property bool windowDragActive: false
    property bool windowPopped: false
    property int windowDragOriginX
    property int windowDragOriginY

    onPressed: {
        if (target.state == "windowed")
        {
            target.state = ""
            return
        }

        target.state = "windowed"
        windowDragOriginX = -1
        windowDragOriginY = -1
        windowDragActive = false
        windowPopped = true
    }

    onReleased: {
        windowDragActive = false
        windowPopped = false
    }

    onPositionChanged: {
        if (!windowPopped)
            return

        if (windowDragOriginX < 0 || windowDragOriginY < 0) {
            /* Must be delayed until the first move due to massively
             * changing coordinates after the popout */
            windowDragOriginX = mouse.x
            windowDragOriginY = mouse.y
        }

        if (!windowDragActive && ((mouse.x - windowDragOriginX) > 5) ||
                (mouse.y - windowDragOriginY) > 5)
        {
            windowDragActive = true;
        }

        if (windowDragActive) {
            target.popoutWindow.moveOffset(mouse.x - windowDragOriginX,
                                           mouse.y - windowDragOriginY)
            windowDragOriginX = mouse.x
            windowDragOriginY = mouse.y
        }
    }
}
