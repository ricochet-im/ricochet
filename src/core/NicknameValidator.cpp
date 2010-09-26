/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Torsion. If not, see http://www.gnu.org/licenses/
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
