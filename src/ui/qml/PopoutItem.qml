/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
