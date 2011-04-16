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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QVector>
#include <QDate>

class QDateTime;
class ContactUser;
class ChatTextWidget;
class ChatTextInput;

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
    void sendMessage(const QString &text);
    void clearUnreadMessages();

signals:
    void messageReceived();
    void unreadMessagesChanged(int unread);

private slots:
    void messageReply();

    void showOfflineNotice();
    void clearOfflineNotice();
    void clearOfflineNoticeInstantly();

    void sendOfflineMessages();

protected:
    virtual bool event(QEvent *event);

private:
    static QHash<ContactUser*,ChatWidget*> userMap;

    ChatTextWidget *textArea;
    ChatTextInput *textInput;
    QWidget *offlineNotice;

    typedef QVector<QPair<QDateTime,QString> > OfflineMessageList;
    OfflineMessageList offlineMessages;

    QDate lastMessageDate;

    int pUnread;
    quint16 lastReceivedID;
    bool useMessageOrdering;

    explicit ChatWidget(ContactUser *user);

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
