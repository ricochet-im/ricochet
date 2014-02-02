import QtQuick 2.0
import org.torsionim.torsion 1.0

Rectangle {
    id: presenceIcon
    width: 16
    height: 4
    radius: 40

    property int status: -1

    onStatusChanged: {
        if (status === ContactUser.Online)
            color = "#3EBB4F"
        else
            color = "darkGray"
    }

    Behavior on color {
        ColorAnimation { }
    }
}

