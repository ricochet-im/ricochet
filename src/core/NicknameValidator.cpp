/* Torsion - http://github.com/special/torsion
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
#include "core/ContactsManager.h"
#include <QToolTip>
#include <QWidget>
#include <QTextDocument>

NicknameValidator::NicknameValidator(QObject *parent)
    : QValidator(parent), m_widget(0), m_validateUnique(false)
{
}

void NicknameValidator::setWidget(QWidget *widget)
{
    m_widget = widget;
}

void NicknameValidator::setValidateUnique(bool unique, ContactUser *exception)
{
    m_validateUnique = unique;
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

    if (m_validateUnique)
    {
        ContactUser *u;
        if ((u = contactsManager->lookupNickname(text)) && u != m_uniqueException)
        {
            showMessage(tr("You already have a contact named <b>%1</b>").arg(Qt::escape(u->nickname())));
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
