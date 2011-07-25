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

Item {
    id: contactListItem
    height: 40
    width: ListView.view.width
    clip: true

    Avatar {
        id: avatar
        x: 17
        anchors.verticalCenter: parent.verticalCenter
        sourceSize: Qt.size(32, 32)
        opacity: (status == ContactUser.Online) ? 1 : 0.7

        sourceContact: contact
    }

    Text {
        id: nickText
        anchors.left: avatar.right
        anchors.leftMargin: 10
        anchors.verticalCenter: avatar.verticalCenter
        text: nickname

        color: (status == ContactUser.Online) ? "black" : "#aeaeae"
        style: (status == ContactUser.Online) ? Text.Normal : Text.Raised
        styleColor: "#f2f2f2"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: setCurrentContact(contact)
    }

    Component.onCompleted: {
        if (currentContact == contact)
            setCurrentContact(contact)
    }

    function intersectsHighlight() {
        var i1 = contactListItem.mapToItem(null, 0, 0).y, i2 = i1+contactListItem.height
        var h1 = contactsView.highlightItem.mapToItem(null, 0, 0).y, h2 = h1+contactsView.highlightItem.height
        return (i2 >= h1 && h2 >= i1)
    }

    states: State {
        name: "current"
        when: contactListItem == currentContactItem && (contactsView.highlightItem.y || true)
              && (contactListItem.y || true) && intersectsHighlight()

        PropertyChanges {
            target: nickText
            color: "white"
            style: Text.Raised
            styleColor: "#777777"
        }

        PropertyChanges {
            target: avatar
            opacity: 1
        }
    }
}
