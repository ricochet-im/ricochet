import QtQuick 2.0
import im.ricochet 1.0

Rectangle {
    id: presenceIcon
    width: 10
    height: 10
    radius: 360

    property int status: -1

    onStatusChanged: {
        if (status === ContactUser.Online)
            color = "#3EBB4F"
        else
            color = "#999999"
    }

    Behavior on color {
        ColorAnimation { }
    }
}

