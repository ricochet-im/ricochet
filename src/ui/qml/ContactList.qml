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

import org.torsionim.torsion 1.0
import Qt 4.7
import "ContactList.js" as Data

Rectangle {
    id: contactList

    width: 187
    color: "#d4d8da"
    clip: true

    property QtObject currentContact
    property Item currentContactItem

    function setCurrentContact(contact) {
        Data.groups[contact.status].currentIndex = contactsView.model.rowOfContact(contact)
        currentContact = contact
        currentContactItem = Data.groups[contact.status].currentItem
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: "#989898"
    }

    ListView {
        id: contactsView
        anchors.fill: parent
        anchors.topMargin: 2
        anchors.bottomMargin: 6
        spacing: 3

        model: contactsModel
        delegate: Qt.createComponent("ContactListGroup.qml")
        footer: Item {
            id: addContactViewPlaceholder
            height: addContactBtn.height + 3

            Component.onCompleted: addContactBtn.viewPlaceholder = addContactViewPlaceholder
        }

        highlight: highlightDelegate
        highlightFollowsCurrentItem: false
    }

    Image {
        id: addContactBtn
        source: "qrc:/icons/plus-subtle.png"
        anchors.left: contactList.left
        anchors.margins: 2
        y: contactsView.contentY, Math.max(contactList.height - height - 2,
                                           parent.mapFromItem(contactsView, 0,
                                                              viewPlaceholder.y - contactsView.contentY)
                                           .y);
        opacity: mouseArea.containsMouse ? 1 : 0.5

        property Item viewPlaceholder

        Behavior on opacity { NumberAnimation { duration: 150 } }

        MouseArea {
            id: mouseArea
            anchors.fill: addContactBtn
            anchors.margins: -4
            hoverEnabled: true

            onClicked: {
                var component = Qt.createComponent("ContactAddDialog.qml")
                var dialog = component.createObject(window)
                dialog.closed.connect(function() { dialog.destroy() })
                dialog.show()
            }
        }
    }

    Component {
        id: highlightDelegate

        Rectangle {
            id: highlight
            width: contactsView.width - 8
            x: currentContactItem.x + 7
            /* Strange usage of the comma operator used to create a binding on the y property */
            y: currentContactItem.y, currentContactItem.ListView.view.y,
               currentContactItem.mapToItem(parent, 0, 0).y
            height: currentContactItem.height

            Behavior on y {
                SmoothedAnimation {
                    velocity: 900
                }
            }

            gradient: Gradient {
                GradientStop { position: 0; color: "#a2c3da"; }
                GradientStop { position: 1; color: "#718898"; }
            }

            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: 1
                anchors.topMargin: 1
                anchors.bottomMargin: 1

                gradient: Gradient {
                    GradientStop { position: 0; color: "#d2dce5"; }
                    GradientStop { position: 1; color: "#8ca4ba"; }
                }
            }
        }
    }

    /* Internal API */
    function _registerGroup(index, item) {
        Data.groups[index] = item
        item.Component.destruction.connect(function() { Data.groups[index] = undefined })
    }
}
