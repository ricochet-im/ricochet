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

#include "main.h"
#include "ChatWidget.h"
#include "ChatTextWidget.h"
#include "core/ContactUser.h"
#include "core/UserIdentity.h"
#include "protocol/ChatMessageCommand.h"
#include "ui/MainWindow.h"
#include "ChatTextInput.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QDateTime>
#include <QTextDocument>
#include <QTextBlock>
#include <QLabel>
#include <QScrollBar>
#include <QApplication>
#include <QFontDialog>
#include <QAction>
#include <QTextDocumentFragment>
#include <QDebug>

#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

QHash<ContactUser*,ChatWidget*> ChatWidget::userMap;

static const int maxMessageChars = 4000;
static const int maxInputLines = 50;

ChatWidget *ChatWidget::widgetForUser(ContactUser *u, bool create)
{
    ChatWidget *widget = userMap.value(u);
    if (!widget && create)
    {
        widget = new ChatWidget(u);
        userMap.insert(u, widget);
    }

    return widget;
}

ChatWidget::ChatWidget(ContactUser *u)
    : user(u), offlineNotice(0), pUnread(0), lastReceivedID(0)
{
    connect(user, SIGNAL(contactDeleted(ContactUser*)), SLOT(deleteLater()));

    /* Peace of mind; verify that makeBlockIdentifier behaves sanely, as i'm unsure
     * of its behavior on big endian machines and have no ability to test. */
    Q_ASSERT(makeBlockIdentifier(LocalUserMessage, 1) != makeBlockIdentifier(RemoteUserMessage, 1));
    Q_ASSERT(makeBlockIdentifier(LocalUserMessage, 1) != makeBlockIdentifier(LocalUserMessage, 2));

    useMessageOrdering = config->value("ui/chatMessageOrdering", true).toBool();

    Q_ASSERT(user);
    connect(user, SIGNAL(connected()), this, SLOT(clearOfflineNotice()));
    connect(user, SIGNAL(connected()), this, SLOT(sendOfflineMessages()));
    connect(user, SIGNAL(disconnected()), this, SLOT(showOfflineNotice()));

    connect(this, SIGNAL(unreadMessagesChanged(int)), user, SLOT(updateStatusLine()));

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    textArea = new ChatTextWidget(this);
    layout->addWidget(textArea);

    createTextInput();
    layout->addWidget(textInput);

    if (!user->isConnected())
        showOfflineNotice();
}

ChatWidget::~ChatWidget()
{
    QHash<ContactUser*,ChatWidget*>::Iterator it = userMap.find(user);
    if (it != userMap.end() && *it == this)
        userMap.erase(it);
}

void ChatWidget::createTextInput()
{
    textInput = new ChatTextInput;
    textInput->setFont(textArea->font());
    textInput->setMaxLength(maxMessageChars);
    textInput->setMaxLines(maxInputLines);
    textInput->setMaximumHeight(250);

    config->addTrackingProperty(QLatin1String("ui/chatFont"), textInput, "font");

    connect(textInput, SIGNAL(textSubmitted(QString)), this, SLOT(sendMessage(QString)));
}

void ChatWidget::sendMessage(const QString &text)
{
    if (text.isEmpty())
        return;

    QDateTime when = QDateTime::currentDateTime();

    if (user->isConnected())
    {
        ChatMessageCommand *command = new ChatMessageCommand;
        connect(command, SIGNAL(commandFinished()), this, SLOT(messageReply()));
        command->send(user->conn(), when, text, lastReceivedID);

        addChatMessage(NULL, command->identifier(), when, text);
    }
    else
    {
        int n = addOfflineMessage(when, text);
        addChatMessage(NULL, (quint16)-1, when, text);
        changeBlockIdentifier(makeBlockIdentifier(LocalUserMessage, (quint16)-1), makeBlockIdentifier(OfflineMessage, n));
    }
}

void ChatWidget::receiveMessage(const QDateTime &when, const QString &text, quint16 messageID, quint16 priorMessageID)
{
    QString message = text.trimmed();
    message.truncate(maxMessageChars);

    lastReceivedID = messageID;
    addChatMessage(user, messageID, when, message, priorMessageID);
    emit messageReceived();

    if (!isVisible() || !window()->isActiveWindow() || window()->isMinimized())
    {
        pUnread++;
        emit unreadMessagesChanged(pUnread);
        qApp->alert(window(), 0);
    }
}

void ChatWidget::clearUnreadMessages()
{
    if (!pUnread)
        return;

    pUnread = 0;
    emit unreadMessagesChanged(pUnread);
}

void ChatWidget::messageReply()
{
    ChatMessageCommand *command = qobject_cast<ChatMessageCommand*>(sender());
    if (!command)
        return;

    if (!isSuccess(command->finalReplyState()))
    {
        /* Connected users are outside the realm of offline messaging */
        if (user->isConnected())
            return;

        int n = addOfflineMessage(command->messageTime(), command->messageText());
        changeBlockIdentifier(makeBlockIdentifier(LocalUserMessage, command->identifier()),
                              makeBlockIdentifier(OfflineMessage, n));
        return;
    }

    /* Update text colors to indicate that the message was received */

    QTextBlock block;
    if (findBlockIdentifier(makeBlockIdentifier(LocalUserMessage, command->identifier()), block))
    {
        /* Loop through the fragments in this block, and see if any have alternate text colors.
         * If they do, set that color. */
        QTextCursor cursor(textArea->document());
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
        {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid())
            {
                QTextCharFormat format = fragment.charFormat();
                if (!format.hasProperty(QTextFormat::UserProperty))
                    continue;

                format.setForeground(QColor(format.property(QTextFormat::UserProperty).value<QRgb>()));

                cursor.setPosition(fragment.position());
                cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
                cursor.setCharFormat(format);
            }
        }
    }
}

void ChatWidget::addChatMessage(ContactUser *from, quint16 messageID, const QDateTime &when, const QString &text,
                                quint16 priorMessage)
{
    QTextCursor cursor;

    if (useMessageOrdering && from && priorMessage)
    {
        /* Position this message after the provided *outgoing* message (found by ID),
         * and after any incoming messages, but before the next outgoing message. */
        QTextBlock priorMessageBlock;
        if (findBlockIdentifier(makeBlockIdentifier(LocalUserMessage, priorMessage), priorMessageBlock))
        {
            /* Go past any previous incoming messages */
            for (;;)
            {
                QTextBlock next = priorMessageBlock.next();
                if (!next.isValid() || blockIdentifierType(next.userState()) == LocalUserMessage)
                    break;
                priorMessageBlock = next;
            }

            cursor = QTextCursor(priorMessageBlock);
            cursor.movePosition(QTextCursor::EndOfBlock);
        }
    }

    if (cursor.isNull())
    {
        cursor = QTextCursor(textArea->document());
        cursor.movePosition(QTextCursor::End);
    }

    cursor.beginEditBlock();

    if (!cursor.atBlockStart())
        cursor.insertBlock();

    if (lastMessageDate.isNull())
        lastMessageDate = QDate::currentDate();

    if (lastMessageDate != when.date())
    {
        lastMessageDate = when.date();

        QTextCharFormat charFormat;
        charFormat.setForeground(QColor(160, 160, 164));
        charFormat.setFontItalic(true);

        cursor.insertText(when.date().toString(QLatin1String("dddd, MMMM d")), charFormat);
        cursor.insertBlock();
    }

    if (messageID)
        cursor.block().setUserState(makeBlockIdentifier(from ? RemoteUserMessage : LocalUserMessage, messageID));

    /* These are the colors used for the timestamp, nickname, and text. Each has two colors;
     * the full color, and the 'faded' color for when the message hasn't been received yet. */
    static const QRgb tsColor[] = { qRgb(160, 160, 164), qRgb(200, 200, 202) };
    static const QRgb myNickColor[] = { qRgb(0, 94, 173), qRgb(135, 185, 227) };
    static const QRgb nickColor[] = { qRgb(174, 0, 0), qRgb(227, 135, 135) };
    static const QRgb textColor[] = { qRgb(0, 0, 0), qRgb(135, 135, 135) };
    
    bool light = (from == 0);

    /* Timestamp */
    QTextCharFormat tsFormat;
    tsFormat.setForeground(QColor(tsColor[light]));
    if (light)
        tsFormat.setProperty(QTextFormat::UserProperty, tsColor[!light]);

    cursor.insertText(when.time().toString(QLatin1String("(HH:mm:ss) ")), tsFormat);

    /* Nickname */
    QTextCharFormat nickFormat;
    nickFormat.setFontWeight(QFont::Bold);
    if (!from)
    {
        nickFormat.setForeground(QColor(myNickColor[light]));
        if (light)
            nickFormat.setProperty(QTextFormat::UserProperty, myNickColor[!light]);
    }
    else
    {
        nickFormat.setForeground(QColor(nickColor[light]));
        if (light)
            nickFormat.setProperty(QTextFormat::UserProperty, nickColor[!light]);
    }

    QString nickname = from ? from->nickname() : user->identity->nickname();

    cursor.insertText(nickname + QLatin1String(": "), nickFormat);

    /* Text */
    QTextCharFormat textFormat;
    textFormat.setForeground(QColor(textColor[light]));
    if (light)
        textFormat.setProperty(QTextFormat::UserProperty, textColor[!light]);

    cursor.insertText(QString(text).replace(QLatin1Char('\n'), QChar::LineSeparator), textFormat);

    cursor.endEditBlock();

    textArea->scrollToBottom();
}

bool ChatWidget::findBlockIdentifier(int identifier, QTextBlock &dst)
{
    if (!identifier)
        return false;

    QTextBlock b = textArea->document()->lastBlock();
    for (;;)
    {
        if (b.userState() == identifier)
        {
            dst = b;
            return true;
        }

        if (b == textArea->document()->begin())
            break;

        b = b.previous();
    }

    return false;
}

bool ChatWidget::changeBlockIdentifier(int oldIdentifier, int newIdentifier)
{
    if (!oldIdentifier || !newIdentifier)
        return false;

    QTextBlock b;
    if (!findBlockIdentifier(oldIdentifier, b))
        return false;

    b.setUserState(newIdentifier);
    return true;
}

void ChatWidget::showOfflineNotice()
{
    if (offlineNotice)
        return;

    /* Create widget */
    offlineNotice = new QWidget;
    offlineNotice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QBoxLayout *layout = new QHBoxLayout(offlineNotice);
    layout->setMargin(0);
    layout->setSpacing(3);
    layout->addStretch();

    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(QLatin1String(":/icons/information.png")));
    layout->addWidget(icon);

    QLabel *message = new QLabel;
    message->setText(tr("<b>%1</b> is not online. Your messages will be delivered as soon as possible.")
                     .arg(Qt::escape(user->nickname())));

    QPalette p = message->palette();
    p.setColor(QPalette::WindowText, Qt::darkGray);
    message->setPalette(p);

    layout->addWidget(message);
    layout->addStretch();

    /* Add to the window */
    QBoxLayout *windowLayout = qobject_cast<QBoxLayout*>(this->layout());
    Q_ASSERT(windowLayout);
    if (!windowLayout)
    {
        delete offlineNotice;
        offlineNotice = 0;
        return;
    }

    windowLayout->insertWidget(1, offlineNotice);

#if QT_VERSION >= 0x040600
    /* Create the size animation */
    offlineNotice->setMaximumHeight(0);

    QPropertyAnimation *ani = new QPropertyAnimation(offlineNotice, QByteArray("maximumHeight"), offlineNotice);
    ani->setEndValue(offlineNotice->sizeHint().height());
    ani->setDuration(300);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
#endif
}

void ChatWidget::clearOfflineNotice()
{
#if QT_VERSION >= 0x040600
    if (!offlineNotice)
        return;

    /* Animate out */
    QPropertyAnimation *ani = new QPropertyAnimation(offlineNotice, QByteArray("maximumHeight"), offlineNotice);
    ani->setEndValue(0);
    ani->setDuration(300);

    connect(ani, SIGNAL(finished()), this, SLOT(clearOfflineNoticeInstantly()));

    ani->start(QAbstractAnimation::DeleteWhenStopped);
#else
    clearOfflineNoticeInstantly();
#endif
}

void ChatWidget::clearOfflineNoticeInstantly()
{
    if (!offlineNotice)
        return;

    offlineNotice->hide();
    offlineNotice->deleteLater();
    offlineNotice = 0;
}

int ChatWidget::addOfflineMessage(const QDateTime &when, const QString &message)
{
    Q_ASSERT(!user->isConnected());
    int n = offlineMessages.size();
    offlineMessages.append(qMakePair(when, message));
    return n;
}

void ChatWidget::sendOfflineMessages()
{
    if (offlineMessages.isEmpty() || !user->isConnected())
        return;

    qDebug() << "Sending" << offlineMessages.size() << "offline messages to" << user->uniqueID;

    /* Clear the original list for the unlikely event that one of the commands fails instantly */
    OfflineMessageList messages = offlineMessages;
    offlineMessages.clear();

    for (int i = 0; i < messages.size(); ++i)
    {
        ChatMessageCommand *command = new ChatMessageCommand;
        connect(command, SIGNAL(commandFinished()), this, SLOT(messageReply()));
        command->send(user->conn(), messages[i].first, messages[i].second);

        changeBlockIdentifier(makeBlockIdentifier(OfflineMessage, i), makeBlockIdentifier(LocalUserMessage, command->identifier()));
    }
}

bool ChatWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Show || event->type() == QEvent::WindowActivate)
    {
        textInput->setFocus(Qt::OtherFocusReason);
        if (pUnread)
            clearUnreadMessages();
    }

    return QWidget::event(event);
}
