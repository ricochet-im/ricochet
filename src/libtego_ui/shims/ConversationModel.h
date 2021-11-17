#pragma once

#include "ContactUser.h"

namespace shims
{
    class ContactUser;
    class ConversationModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_ENUMS(MessageStatus)

        Q_PROPERTY(shims::ContactUser* contact READ contact WRITE setContact NOTIFY contactChanged)
        Q_PROPERTY(int unreadCount READ getUnreadCount RESET resetUnreadCount NOTIFY unreadCountChanged)
        Q_PROPERTY(int conversationEventCount READ getConversationEventCount NOTIFY conversationEventCountChanged)
    public:
        ConversationModel(QObject *parent = 0);

        enum {
            TimestampRole = Qt::UserRole,
            IsOutgoingRole,
            StatusRole,
            SectionRole,
            TimespanRole,
            TypeRole,
            TransferRole,
        };

        enum MessageStatus {
            None,
            Received,
            Queued,
            Sending,
            Delivered,
            Error
        };

        enum MessageDataType
        {
            InvalidMessage = -1,
            TextMessage,
            TransferMessage,
        };

        enum TransferStatus
        {
            InvalidTransfer,
            Pending,
            Accepted,
            Rejected,
            InProgress,
            Cancelled,
            Finished,
            UnknownFailure,
            BadFileHash,
            NetworkError,
            FileSystemError,
        };
        Q_ENUM(TransferStatus);

        enum TransferDirection
        {
            InvalidDirection,
            Uploading,
            Downloading,
        };
        Q_ENUM(TransferDirection);

        enum EventType {
            InvalidEvent,
            TextMessageEvent,
            TransferMessageEvent,
            UserStatusUpdateEvent
        };

        enum UserStatusTarget {
            UserTargetNone,
            UserTargetClient,
            UserTargetPeer
        };

        // impl QAbstractListModel
        virtual QHash<int,QByteArray> roleNames() const;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        shims::ContactUser *contact() const;
        void setContact(shims::ContactUser *contact);
        int getUnreadCount() const;
        Q_INVOKABLE void resetUnreadCount();

        void sendFile();
        bool hasEventsToExport();
        Q_INVOKABLE int getConversationEventCount() const { return this->events.size(); }
        bool exportConversation();
        // invokable function neeeds to use a Qt type since it is invokable from QML
        static_assert(std::is_same_v<quint32, tego_file_transfer_id_t>);
        Q_INVOKABLE void tryAcceptFileTransfer(quint32 id);
        Q_INVOKABLE void cancelFileTransfer(quint32 id);
        Q_INVOKABLE void rejectFileTransfer(quint32 id);


        void setStatus(ContactUser::Status status);

        void fileTransferRequestReceived(tego_file_transfer_id_t id, QString fileName, QString fileHash, quint64 fileSize);
        void fileTransferRequestAcknowledged(tego_file_transfer_id_t id, bool accepted);
        void fileTransferRequestResponded(tego_file_transfer_id_t id, tego_file_transfer_response_t response);
        void fileTransferRequestProgressUpdated(tego_file_transfer_id_t id, quint64 bytesTransferred);
        void fileTransferRequestCompleted(tego_file_transfer_id_t id, tego_file_transfer_result_t result);

        void messageReceived(tego_message_id_t messageId, QDateTime timestamp, const QString& text);
        void messageAcknowledged(tego_message_id_t messageId, bool accepted);

    public slots:
        void sendMessage(const QString &text);
        void clear();

    signals:
        void contactChanged();
        void unreadCountChanged(int prevCount, int currentCount);
        void conversationEventCountChanged();
    private:
        void setUnreadCount(int count);

        shims::ContactUser* contactUser = nullptr;

        struct MessageData
        {
            MessageDataType type = InvalidMessage;
            QString text = {};
            QDateTime time = {};
            static_assert(std::is_same_v<quint32, tego_file_transfer_id_t>);
            static_assert(std::is_same_v<quint32, tego_message_id_t>);
            quint32 identifier = 0;
            MessageStatus status = None;
            quint8 attemptCount = 0;
            // file transfer data
            QString fileName = {};
            qint64 fileSize = 0;
            QString fileHash = {};
            quint64 bytesTransferred = 0;
            TransferDirection transferDirection = InvalidDirection;
            TransferStatus transferStatus = InvalidTransfer;
        };

        struct EventData
        {
            EventType type = InvalidEvent;
            union {
                struct {
                    size_t reverseIndex = 0;
                } messageData;
                struct {
                    size_t reverseIndex = 0;
                    TransferStatus status = InvalidTransfer;
                    qint64 bytesTransferred = 0; // we care about this for when a transfer is cancelled midway 
                } transferData;
                struct {
                    ContactUser::Status status = ContactUser::Status::Offline;
                    UserStatusTarget target = UserTargetNone; // when the protocol is eventually fixed and users
                                                              // are notified of being blocked, this will be needed
                } userStatusData;
            };
            QDateTime time = {};

            EventData() {}
        };

        QList<MessageData> messages;
        QList<EventData> events;

        void addEventFromMessage(int row);

        void deserializeTextMessageEventToFile(const EventData &event, std::ofstream &ofile) const;
        void deserializeTransferMessageEventToFile(const EventData &event, std::ofstream &ofile) const;
        void deserializeUserStatusUpdateEventToFile(const EventData &event, std::ofstream &ofile) const;
        void deserializeEventToFile(const EventData &event, std::ofstream &ofile) const;

        int unreadCount = 0;

        void emitDataChanged(int row);

        int indexOfMessage(quint32 identifier) const;
        int indexOfOutgoingMessage(quint32 identifier) const;
        int indexOfIncomingMessage(quint32 identifier) const;

        static const char* getMessageStatusString(const MessageStatus status);
        static const char* getTransferStatusString(const TransferStatus status);
    };
}
