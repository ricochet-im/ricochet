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
