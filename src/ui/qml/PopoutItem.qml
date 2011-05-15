import Qt 4.7

Item {
    id: popout

    property QtObject popoutWindow
    property bool restoreOnWindowClose: true

    /* Reanchoring hack (see AnchorChanges) when this item is the root
     * of a component created by Loader. Qt bug as of 4.7.2 */
    /* XXXXXX Should the loader be in the popout item, instead of all of this mess? */
    property bool itemIsFromLoader: false

    states: State {
        name: "windowed"

        ParentChange {
            target: popout
            /* XXX we assume here and below that the root item is 'window' */
            parent: window
        }

        StateChangeScript {
            name: "createWindow"
            script: {
                var scenePos = popout.mapToItem(null, 0, 0)
                popoutWindow = popoutManager.createWindow(Qt.rect(scenePos.x, scenePos.y,
                                                                  width, height))
                x = popoutWindow.sceneX
                y = popoutWindow.sceneY
            }
        }

        AnchorChanges {
            target: itemIsFromLoader ? popout.parent : popout
            anchors.top: undefined
            anchors.bottom: undefined
            anchors.verticalCenter: undefined
            anchors.left: undefined
            anchors.right: undefined
            anchors.horizontalCenter: undefined
            anchors.baseline: undefined
        }

        PropertyChanges {
            target: popout
            width: popoutWindow ? popoutWindow.width : width
            height: popoutWindow ? popoutWindow.height : height
        }
    }

    transitions: [
        Transition {
            to: "windowed"
            reversible: false

            SequentialAnimation {
                ScriptAction { scriptName: "createWindow" }
                AnchorAnimation { duration: 0 }
                ParentAnimation { }
                PropertyAction { target: popout; properties: "width,height" }
            }
        },
        Transition {
            from: "windowed"
            reversible: false

            SequentialAnimation {
                ScriptAction {
                    script: if (popoutWindow !== null) popoutWindow.close()
                }

                ParentAnimation { }
                ScriptAction { script: console.log(popout.parent) }
                AnchorAnimation { duration: 0 }
            }
        }
    ]

    Connections {
        target: popoutWindow
        onClosed: {
            popoutWindow = null
            if (restoreOnWindowClose)
                state = ""
        }
    }
}
