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

ListView {
    id: contactsGroupView
    spacing: 3
    height: 1 /* See autoSize() */
    width: contactsView.width
    interactive: false /* disable FLickable; scrolling takes place in the parent view */

    function autoSize() {
        /* Size the view to fit its contents exactly. 19 is the height of the header item. */
        height = (count ? contentHeight : 0) + 19
    }

    onContentHeightChanged: autoSize()
    onCountChanged: autoSize()

    Connections {
        target: contactsModel
        onRowsInserted: myModel.forceUpdateRoot()
        onRowsRemoved: myModel.forceUpdateRoot()
        onRowsMoved: myModel.forceUpdateRoot()
    }

    VisualDataModel {
        id: myModel
        model: contactsModel
        delegate: Qt.createComponent("ContactListItem.qml")

        function forceUpdateRoot() {
            rootIndex = parentModelIndex()
            rootIndex = modelIndex(index)
        }

        Component.onCompleted: {
            rootIndex = modelIndex(index)
        }
    }

    model: myModel

    header: groupHeader

    Component {
        id: groupHeader

        Item {
            width: ListView.view.width
            height: 19

            Text {
                id: title
                text: nickname
                x: 16
                y: 0
                color: "#6e7f94"
                font.pixelSize: 13
                font.bold: true
            }
        }
    }

    Connections {
        target: contactList
        onCurrentContactItemChanged: {
            if (currentContactItem !== currentItem)
                contactsGroupView.currentIndex = -1
        }
    }
}
