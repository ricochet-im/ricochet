/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, Robin Burchell <robin.burchell@collabora.co.uk>
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

#ifndef CONTACTADDDIALOG_H
#define CONTACTADDDIALOG_H

#include <QDialog>

class QLineEdit;
class FancyTextEdit;

class ContactAddDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ContactAddDialog(QWidget *parent = 0);
private slots:
    void processFriendAdd();
    void checkClipboardForId();
private:
    QWidget *createUI() const;
    QLineEdit * const m_nickname;
    QLineEdit * const m_id;
    FancyTextEdit * const m_message;
};

#endif // CONTACTADDDIALOG_H
