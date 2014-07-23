#ifndef CONVERSATIONMODEL_H
#define CONVERSATIONMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include "core/ContactUser.h"

class ConversationModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(MessageStatus)

    Q_PROPERTY(ContactUser* contact READ contact WRITE setContact NOTIFY contactChanged)
    Q_PROPERTY(int unreadCount READ unreadCount RESET resetUnreadCount NOTIFY unreadCountChanged)

public:
    enum {
        TimestampRole = Qt::UserRole,
        IsOutgoingRole,
        StatusRole,
        SectionRole
    };

    enum MessageStatus {
        Received,
        Sending,
        Delivered,
        Error
    };

    ConversationModel(QObject *parent = 0);

    ContactUser *contact() const { return m_contact; }
    void setContact(ContactUser *contact);

    int unreadCount() const { return m_unreadCount; }
    Q_INVOKABLE void resetUnreadCount();

    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

public slots:
    void sendMessage(const QString &text);
    void clear();

signals:
    void contactChanged();
    void unreadCountChanged();

private slots:
    void receiveMessage(const ChatMessageData &message);
    void messageReply();
    void onContactStatusChanged();

private:
    struct MessageData {
        QString text;
        QDateTime time;
        quint16 identifier;
        MessageStatus status;
    };

    ContactUser *m_contact;
    QList<MessageData> messages;
    quint16 lastReceivedId;
    int m_unreadCount;

    int indexOfIdentifier(quint16 identifier, bool isOutgoing) const;
};

#endif

