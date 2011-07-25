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

#include "NicknameValidator.h"
#include "core/UserIdentity.h"
#include <QToolTip>
#include <QWidget>
#include <QTextDocument>

NicknameValidator::NicknameValidator(QObject *parent)
    : QValidator(parent), m_widget(0), m_uniqueIdentity(0), m_uniqueException(0)
{
}

void NicknameValidator::setWidget(QWidget *widget)
{
    m_widget = widget;
}

void NicknameValidator::setValidateUnique(UserIdentity *identity, ContactUser *exception)
{
    m_uniqueIdentity = identity;
    m_uniqueException = exception;
}

void NicknameValidator::fixup(QString &text) const
{
    text = text.trimmed();
}

QValidator::State NicknameValidator::validate(QString &text, int &pos) const
{
    Q_UNUSED(pos);

    if (text.size() < 1)
        return Intermediate;
    else if (text.size() > 15)
        return Invalid;

    bool nonws = false, wssuf = false;
    for (QString::iterator it = text.begin(); it != text.end(); ++it)
    {
        if (!it->isPrint())
            return Invalid;

        if (it->isSpace())
        {
            if (!nonws)
                return Invalid;
            wssuf = true;
        }
        else
        {
            nonws = true;
            wssuf = false;
        }
    }

    if (!nonws)
        return Invalid;
    else if (wssuf)
        return Intermediate;

    if (m_uniqueIdentity)
    {
        ContactUser *u;
        if (((u = m_uniqueIdentity->contacts.lookupNickname(text)) && u != m_uniqueException)
            || QString::compare(text, m_uniqueIdentity->nickname(), Qt::CaseInsensitive) == 0)
        {
            showMessage(tr("You already have a contact named <b>%1</b>").arg(Qt::escape(text)));
            return Intermediate;
        }
    }

    if (m_widget && m_widget->hasFocus())
        QToolTip::hideText();

    return Acceptable;
}

void NicknameValidator::showMessage(const QString &message) const
{
    if (!m_widget || !m_widget->hasFocus())
        return;

    QToolTip::showText(m_widget->mapToGlobal(QPoint(0,0)), message, m_widget);
}
