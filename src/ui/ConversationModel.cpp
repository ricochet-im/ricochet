#include "ConversationModel.h"
#include "protocol/ChatMessageCommand.h"

ConversationModel::ConversationModel(QObject *parent)
    : QAbstractListModel(parent), m_contact(0)
{
    roles[Qt::DisplayRole] = "text";
    roles[TimestampRole] = "timestamp";
    roles[IsOutgoingRole] = "isOutgoing";
}

void ConversationModel::setContact(ContactUser *contact)
{
    if (contact == m_contact)
        return;

    beginResetModel();
    messages.clear();

    if (m_contact)
        disconnect(m_contact, 0, this, 0);
    m_contact = contact;
    if (m_contact) {
        connect(m_contact, SIGNAL(incomingChatMessage(ChatMessageData)), this,
                SLOT(receiveMessage(ChatMessageData)));
    }

    endResetModel();
    emit contactChanged();
}

void ConversationModel::sendMessage(const QString &text)
{
    if (text.isEmpty())
        return;

    ChatMessageCommand *command = new ChatMessageCommand;
    // connect(command, SIGNAL(commandFinished()), this, SLOT(messageReply()));
    // XXX lastReceivedId
    command->send(m_contact->conn(), QDateTime::currentDateTime(), text, 0);

    beginInsertRows(QModelIndex(), messages.size(), messages.size());
    MessageData message = { text, QDateTime::currentDateTime(), true };
    messages.append(message);
    endInsertRows();
}

void ConversationModel::receiveMessage(const ChatMessageData &data)
{
    beginInsertRows(QModelIndex(), messages.size(), messages.size());
    MessageData message = { data.text.trimmed(), data.when, false };
    messages.append(message);
    endInsertRows();
}

QHash<int,QByteArray> ConversationModel::roleNames() const
{
    return roles;
}

int ConversationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return messages.size();
}

QVariant ConversationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= messages.size())
        return QVariant();

    const MessageData &message = messages[index.row()];

    switch (role) {
        case Qt::DisplayRole: return message.text;
        case TimestampRole: return message.time;
        case IsOutgoingRole: return message.isOutgoing;
    }

    return QVariant();
}

