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

#ifndef NICKNAMEVALIDATOR_H
#define NICKNAMEVALIDATOR_H

#include <QValidator>

class QWidget;
class ContactUser;
class UserIdentity;

class NicknameValidator : public QValidator
{
    Q_OBJECT
    Q_DISABLE_COPY(NicknameValidator)

public:
    explicit NicknameValidator(QObject *parent = 0);

    void setWidget(QWidget *widget);
    void setValidateUnique(UserIdentity *identity, ContactUser *exception = 0);

    virtual State validate(QString &text, int &pos) const;

private:
    QWidget *m_widget;
    UserIdentity *m_uniqueIdentity;
    ContactUser *m_uniqueException;

    void showMessage(const QString &message) const;
};

#endif // NICKNAMEVALIDATOR_H
