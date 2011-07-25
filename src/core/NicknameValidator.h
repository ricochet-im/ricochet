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

    Q_PROPERTY(UserIdentity* uniqueToIdentity READ uniqueToIdentity WRITE setValidateUnique)

public:
    explicit NicknameValidator(QObject *parent = 0);

    UserIdentity *uniqueToIdentity() const { return m_uniqueIdentity; }

    void setWidget(QWidget *widget);
    void setValidateUnique(UserIdentity *identity, ContactUser *exception = 0);

    virtual void fixup(QString &) const;
    virtual State validate(QString &text, int &pos) const;

private:
    QWidget *m_widget;
    UserIdentity *m_uniqueIdentity;
    ContactUser *m_uniqueException;

    void showMessage(const QString &message) const;
};

#endif // NICKNAMEVALIDATOR_H
