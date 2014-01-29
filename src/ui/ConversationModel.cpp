#include "ConversationModel.h"
#include "protocol/ChatMessageCommand.h"

ConversationModel::ConversationModel(QObject *parent)
    : QAbstractListModel(parent), m_contact(0), lastReceivedId(0)
{
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
    connect(command, SIGNAL(commandFinished()), this, SLOT(messageReply()));
    command->send(m_contact->conn(), QDateTime::currentDateTime(), text, lastReceivedId);

    beginInsertRows(QModelIndex(), messages.size(), messages.size());
    MessageData message = { text, QDateTime::currentDateTime(), command->identifier(), Sending };
    messages.append(message);
    endInsertRows();
}

void ConversationModel::receiveMessage(const ChatMessageData &data)
{
    // If priorMessageID is non-zero, it represents the identifier of the last message
    // the peer had received when this message was sent. To help keep the flow of a
    // conversation despite latency, attempt to insert this message where the peer sees it.
    int row = messages.size();
    if (data.priorMessageID) {
        for (row--; row >= 0; row--) {
            if (messages[row].status == Received ||
                messages[row].identifier == data.priorMessageID ||
                messages.size() - row >= 5)
            {
                break;
            }
        }
        row++;
    }

    beginInsertRows(QModelIndex(), row, row);
    MessageData message = { data.text.trimmed(), data.when, data.messageID, Received };
    lastReceivedId = data.messageID;
    messages.insert(row, message);
    endInsertRows();
}

void ConversationModel::messageReply()
{
    ChatMessageCommand *command = qobject_cast<ChatMessageCommand*>(sender());
    if (!command)
        return;

    int row = indexOfIdentifier(command->identifier(), true);
    if (row < 0)
        return;

    MessageData &data = messages[row];
    data.status = isSuccess(command->finalReplyState()) ? Delivered : Error;
    emit dataChanged(index(row, 0), index(row, 0));
}

QHash<int,QByteArray> ConversationModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "text";
    roles[TimestampRole] = "timestamp";
    roles[IsOutgoingRole] = "isOutgoing";
    roles[StatusRole] = "status";
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
        case IsOutgoingRole: return message.status != Received;
        case StatusRole: return message.status;
    }

    return QVariant();
}

int ConversationModel::indexOfIdentifier(quint16 identifier, bool isOutgoing) const
{
    for (int i = messages.size() - 1; i >= 0; i--) {
        if (messages[i].identifier == identifier && (messages[i].status != Received) == isOutgoing)
            return i;
    }
    return -1;
}

