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
#include "ChatTextWidget.h"
#include "core/UserIdentity.h"
#include "protocol/ChatMessageCommand.h"
#include <QScrollBar>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QFontDialog>
#include <QApplication>
#include <QWindowsVistaStyle>
#include <QShortcut>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QDebug>

static const int defaultBacklog = 125;

ChatTextWidget::ChatTextWidget(ContactUser *u, QWidget *parent)
    : QTextEdit(parent), user(u)
{
    Q_ASSERT(user);

    useMessageOrdering = config->value("ui/chatMessageOrdering", true).toBool();
    setReadOnly(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(static_cast<Qt::FocusPolicy>(focusPolicy() & ~Qt::TabFocus));
    document()->setMaximumBlockCount(config->value("ui/chatBacklog", defaultBacklog).toInt());
    connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), SLOT(scrollToBottom()));

    connect(user, SIGNAL(incomingChatMessage(ChatMessageData)), SLOT(receiveMessage(ChatMessageData)));
    connect(user, SIGNAL(outgoingChatMessage(ChatMessageData,ChatMessageCommand*)),
            SLOT(outgoingMessage(ChatMessageData,ChatMessageCommand*)));
    connect(user, SIGNAL(connected()), this, SLOT(sendOfflineMessages()));

#ifdef Q_OS_WIN
    if (qobject_cast<QWindowsVistaStyle*>(style()))
    {
        QFont defaultFont(QLatin1String("Segoe UI, MS Shell Dlg 2"), 9);
        setFont(config->value("ui/chatFont", defaultFont).value<QFont>());
    }
    else
#endif
        setFont(config->value("ui/chatFont", QApplication::font(this)).value<QFont>());
    config->addTrackingProperty(QLatin1String("ui/chatFont"), this, "font");

    new QShortcut(QKeySequence(Qt::Key_F10), this, SLOT(clear()));
}

void ChatTextWidget::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatTextWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu(e->pos());
    menu->addAction(tr("Clear"), this, SLOT(clear()), QKeySequence(Qt::Key_F10));
    menu->addSeparator();
    menu->addAction(tr("Change Font"), this, SLOT(showFontDialog()));

    menu->exec(e->globalPos());
    delete menu;
}

bool ChatTextWidget::event(QEvent *e)
{
    bool re = QTextEdit::event(e);

    if (e->type() == QEvent::FontChange)
        emit fontChanged(font());

    return re;
}

void ChatTextWidget::showFontDialog()
{
    QFont newFont = QFontDialog::getFont(0, font());
    config->setValue("ui/chatFont", newFont);
    setFont(newFont);
}

void ChatTextWidget::receiveMessage(const ChatMessageData &message)
{
    addChatMessage(user, message.messageID, message.when, message.text, message.priorMessageID);
}

void ChatTextWidget::outgoingMessage(const ChatMessageData &message, ChatMessageCommand *command)
{
    connect(command, SIGNAL(commandFinished()), SLOT(outgoingMessageReply()));
    addChatMessage(NULL, message.messageID, message.when, message.text, message.priorMessageID);
}

void ChatTextWidget::outgoingMessageReply()
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
        QTextCursor cursor(document());
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

void ChatTextWidget::addChatMessage(ContactUser *from, quint16 messageID, const QDateTime &when,
                                    const QString &text, quint16 priorMessage)
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
        cursor = QTextCursor(document());
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

    scrollToBottom();
}

bool ChatTextWidget::findBlockIdentifier(int identifier, QTextBlock &dst)
{
    if (!identifier)
        return false;

    QTextBlock b = document()->lastBlock();
    for (;;)
    {
        if (b.userState() == identifier)
        {
            dst = b;
            return true;
        }

        if (b == document()->begin())
            break;

        b = b.previous();
    }

    return false;
}

bool ChatTextWidget::changeBlockIdentifier(int oldIdentifier, int newIdentifier)
{
    if (!oldIdentifier || !newIdentifier)
        return false;

    QTextBlock b;
    if (!findBlockIdentifier(oldIdentifier, b))
        return false;

    b.setUserState(newIdentifier);
    return true;
}

int ChatTextWidget::addOfflineMessage(const QDateTime &when, const QString &message)
{
    Q_ASSERT(!user->isConnected());
    int n = offlineMessages.size();
    offlineMessages.append(qMakePair(when, message));
    return n;
}

void ChatTextWidget::sendOfflineMessages()
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

        changeBlockIdentifier(makeBlockIdentifier(OfflineMessage, i),
                              makeBlockIdentifier(LocalUserMessage, command->identifier()));
    }
}
