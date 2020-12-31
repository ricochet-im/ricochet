#pragma once

namespace shims
{
    class ContactUser;
    class ConversationModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_ENUMS(MessageStatus)

        Q_PROPERTY(shims::ContactUser* contact READ contact WRITE setContact NOTIFY contactChanged)
        Q_PROPERTY(int unreadCount READ getUnreadCount RESET resetUnreadCount NOTIFY unreadCountChanged)
    public:
        ConversationModel(QObject *parent = 0);

        enum {
            TimestampRole = Qt::UserRole,
            IsOutgoingRole,
            StatusRole,
            SectionRole,
            TimespanRole
        };

        enum MessageStatus {
            None,
            Received,
            Queued,
            Sending,
            Delivered,
            Error
        };

        // impl QAbstractListModel
        virtual QHash<int,QByteArray> roleNames() const;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        shims::ContactUser *contact() const;
        void setContact(shims::ContactUser *contact);
        int getUnreadCount() const;
        Q_INVOKABLE void resetUnreadCount();

        void messageReceived(tego_message_id_t messageId, QDateTime timestamp, const QString& text);
        void messageAcknowledged(tego_message_id_t messageId, bool accepted);
    public slots:
        void sendMessage(const QString &text);
        void clear();

    signals:
        void contactChanged();
        void unreadCountChanged(int prevCount, int currentCount);
    private:
        void setUnreadCount(int count);

        shims::ContactUser* contactUser = nullptr;

        struct MessageData
        {
            QString text = {};
            QDateTime time = {};
            tego_message_id_t identifier = 0;
            MessageStatus status = None;
            quint8 attemptCount = 0;
        };

        QList<MessageData> messages;
        int unreadCount = 0;

        int indexOfIdentifier(tego_message_id_t messageId, bool isOutgoing) const;
    };
}