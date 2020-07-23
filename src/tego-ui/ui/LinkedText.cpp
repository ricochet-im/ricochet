/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

#include "LinkedText.h"
#include <QRegularExpression>
#include <QUrl>
#include <QClipboard>
#include <QGuiApplication>
#include <QDebug>

LinkedText::LinkedText(QObject *parent)
    : QObject(parent)
{
    // Select things that look like URLs of some kind and allow QUrl::fromUserInput to validate them
    linkRegex = QRegularExpression(QStringLiteral("([a-z]{3,9}:|www\\.)([^\\s,.);!>]|[,.);!>](?!\\s|$))+"), QRegularExpression::CaseInsensitiveOption);

    allowedSchemes << QStringLiteral("http")
                   << QStringLiteral("https")
                   << QStringLiteral("torsion")
                   << QStringLiteral("ricochet");
}

QString LinkedText::parsed(const QString &input)
{
    QString re;
    int p = 0;
    QRegularExpressionMatchIterator it = linkRegex.globalMatch(input);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart();

        QUrl url = QUrl::fromUserInput(match.capturedRef().toString());
        if (!allowedSchemes.contains(url.scheme().toLower()))
            continue;

        if (start > p)
            re.append(input.mid(p, start - p).toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>")));
        re.append(QStringLiteral("<a href=\"%1\">%2</a>").arg(QString::fromLatin1(url.toEncoded()).toHtmlEscaped(), match.capturedRef().toString().toHtmlEscaped()));
        p = match.capturedEnd();
    }

    if (p < input.size())
        re.append(input.mid(p).toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>")));

    return re;
}

void LinkedText::copyToClipboard(const QString &text)
{
    QClipboard *clipboard = qApp->clipboard();
    clipboard->setText(text);
    // For X11, also copy to the selection clipboard (middle-click)
    if (clipboard->supportsSelection())
        clipboard->setText(text, QClipboard::Selection);
}

