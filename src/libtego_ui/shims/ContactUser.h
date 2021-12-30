#pragma once

#include "utils/Settings.h"

namespace shims
{
    class ContactsManager;
    class ConversationModel;
    class OutgoingContactRequest;
    class ContactUser : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(ContactUser)
        Q_ENUMS(Status)

        Q_PROPERTY(QString nickname READ getNickname WRITE setNickname NOTIFY nicknameChanged)
        Q_PROPERTY(QString contactID READ getContactID CONSTANT)
        Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
        Q_PROPERTY(shims::OutgoingContactRequest* contactRequest READ contactRequest NOTIFY statusChanged)
        Q_PROPERTY(shims::ConversationModel* conversation READ conversation CONSTANT)
    public:
        enum Status
        {
            Online,
            Offline,
            RequestPending,
            RequestRejected,
            Outdated
        };

        ContactUser(const QString& serviceId, const QString& nickname);

        QString getNickname() const;
        QString getContactID() const;
        Status getStatus() const;
        void setStatus(Status status);
        shims::OutgoingContactRequest *contactRequest();
        shims::ConversationModel *conversation();

        Q_INVOKABLE void deleteContact();
        Q_INVOKABLE void sendFile();
        Q_INVOKABLE bool exportConversation();

        std::unique_ptr<tego_user_id_t> toTegoUserId() const;

    public slots:
        void setNickname(const QString &nickname);

    signals:
        void nicknameChanged();
        void statusChanged();
        void contactDeleted(shims::ContactUser *user);

    private:
        shims::ConversationModel* conversationModel;
        shims::OutgoingContactRequest* outgoingContactRequest;

        Status status;
        QString serviceId;
        QString nickname;

        SettingsObject settings;

        friend class shims::ContactsManager;
        friend class shims::ConversationModel;
    };
}