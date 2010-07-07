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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QVector>
#include <QDate>

class QDateTime;
class ContactUser;

class ChatWidget : public QWidget
{
Q_OBJECT
public:
    ContactUser * const user;

    static ChatWidget *widgetForUser(ContactUser *user, bool create = true);

    ~ChatWidget();

    void receiveMessage(const QDateTime &when, const QString &text, quint16 messageID = 0, quint16 lastSeenID = 0);

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

    void sendOfflineMessages();

protected:
    virtual bool event(QEvent *event);

private:
    static QHash<ContactUser*,ChatWidget*> userMap;

    class QTextEdit *textArea;
    class QLineEdit *textInput;
    class QWidget *offlineNotice;

    typedef QVector<QPair<QDateTime,QString> > OfflineMessageList;
    OfflineMessageList offlineMessages;

    QDate lastMessageDate;

    int pUnread;
    quint16 lastReceivedID;
    bool useMessageOrdering;

    explicit ChatWidget(ContactUser *user);

    void createTextArea();
    void createTextInput();

    void addChatMessage(ContactUser *user, quint16 messageID, const QDateTime &when, const QString &text,
                              quint16 priorMessage = 0);

    /* The upper 16 bits of a text block identifier hold the message type; the lower 16 are a type-specific identifier */
    enum MessageType
    {
        LocalUserMessage = 1,
        RemoteUserMessage,
        OfflineMessage
    };

    int makeBlockIdentifier(MessageType type, quint16 messageid) const
    {
        return int((unsigned(type) << 16) | unsigned(messageid));
    }
    MessageType blockIdentifierType(int identifier) const
    {
        return static_cast<MessageType>(unsigned(identifier) >> 16);
    }

    bool findBlockIdentifier(int identifier, class QTextBlock &block);
    bool changeBlockIdentifier(int oldIdentifier, int newIdentifier);

    int addOfflineMessage(const QDateTime &when, const QString &message);
};

#endif // CHATWIDGET_H
