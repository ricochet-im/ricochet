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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>

class QDateTime;
class ContactUser;

class ChatWidget : public QWidget
{
Q_OBJECT
public:
    ContactUser * const user;

    static ChatWidget *widgetForUser(ContactUser *user, bool create = true);

    ~ChatWidget();

    void receiveMessage(const QDateTime &when, const QString &text);

    int unreadMessages() const { return pUnread; }

public slots:
    void clearUnreadMessages();

signals:
    void messageReceived();
    void unreadMessagesChanged(int unread);

private slots:
    void sendInputMessage();
    void messageReply();

    void scrollToBottom();

    void showOfflineNotice();
    void clearOfflineNotice();
    void clearOfflineNoticeInstantly();

protected:
    virtual bool event(QEvent *event);

private:
    static QHash<ContactUser*,ChatWidget*> userMap;

    class QTextEdit *textArea;
    class QLineEdit *textInput;
    class QWidget *offlineNotice;

    int pUnread;

    explicit ChatWidget(ContactUser *user);

    void createTextArea();
    void createTextInput();

    void addChatMessage(ContactUser *user, const QDateTime &when, const QString &text, int identifier = 0);
    bool findBlockIdentifier(int identifier, class QTextBlock &block);
};

#endif // CHATWIDGET_H
