/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
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
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#ifndef CONTACTIDVALIDATOR_H
#define CONTACTIDVALIDATOR_H

#include <QValidator>

class ContactIDValidator : public QValidator
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactIDValidator)

public:
    ContactIDValidator(QObject *parent = 0);

    static bool isValidID(const QString &text);
    static QString hostnameFromID(const QString &ID);
    static QString idFromHostname(const QString &hostname);
    static QString idFromHostname(const QByteArray &hostname) { return idFromHostname(QString::fromLatin1(hostname)); }

    virtual State validate(QString &text, int &pos) const;
};

#endif // CONTACTIDVALIDATOR_H
