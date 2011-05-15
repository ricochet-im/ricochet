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

PopoutItem {
    id: contactPage

    itemIsFromLoader: false

    property QtObject contact

    Rectangle {
        id: contactHeader
        anchors.top: contactPage.top
        anchors.left: contactPage.left
        anchors.right: contactPage.right
        height: 97

        gradient: Gradient {
            GradientStop { position: 0; color: "#eeeeee"; }
            GradientStop { position: 1; color: "#d3d3d3"; }
        }

        Avatar {
            id: avatar
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 8
            width: height

            source: contact.avatarPath
        }

        Column {
            anchors.left: avatar.right
            anchors.leftMargin: 8
            anchors.top: avatar.top

            Text {
                font.bold: true
                font.pixelSize: 13
                text: contact.nickname
            }

            Text {
                color: "#a5a5a5"
                text: contact.contactID
                font.family: "Courier New"
                font.pixelSize: 10
            }
        }

        Image {
            id: popoutButton
            anchors.right: contactHeader.right
            anchors.rightMargin: 3
            anchors.top: contactHeader.top
            anchors.topMargin: 2
            source: "popout.png"
            opacity: 0.3

            PopoutClickArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true

                target: contactPage
            }

            states: State {
                name: "mouseOver"
                when: mouseArea.containsMouse || mouseArea.windowPopped

                PropertyChanges {
                    target: popoutButton
                    opacity: 1.0
                }
            }

            transitions: Transition {
                to: "mouseOver"
                reversible: true

                NumberAnimation {
                    target: popoutButton
                    property: "opacity"
                    duration: 300
                }
            }
        }

        MouseArea {
            anchors.verticalCenter: contactHeader.bottom
            anchors.left: contactHeader.left
            anchors.right: contactHeader.right
            height: 10

            onPositionChanged: {
                contactHeader.height = Math.max(48, Math.min(mouse.y+contactHeader.height, 97))
            }
        }
    }

    Rectangle {
        id: headerSeparator
        anchors.bottom: contactHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "#989898"
    }

    ChatArea {
        id: chat

        anchors.top: headerSeparator.bottom
        anchors.left: contactPage.left
        anchors.right: contactPage.right
        anchors.bottom: contactPage.bottom
    }
}
