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

#ifndef CONTACTREQUESTDIALOG_H
#define CONTACTREQUESTDIALOG_H

#include <QDialog>

class IncomingContactRequest;

class ContactRequestDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactRequestDialog)

public:
    enum ExtendedDialogCode
    {
        Cancelled = -1
    };

    IncomingContactRequest * const request;

    explicit ContactRequestDialog(IncomingContactRequest *request, QWidget *parent = 0);

    virtual void accept();
    virtual void reject();

public slots:
    void rejectRequest();

private:
    class QLineEdit *m_nickname;
};

#endif // CONTACTREQUESTDIALOG_H
