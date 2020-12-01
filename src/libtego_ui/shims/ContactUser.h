#pragma once

class ContactUser;
class OutgoingContactRequest;
class ConversationModel;

namespace shims
{
    class ContactsManager;
    class ConversationModel;
    class ContactUser : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(ContactUser)
        Q_ENUMS(Status)

        Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
        Q_PROPERTY(QString contactID READ contactID CONSTANT)
        Q_PROPERTY(Status status READ status NOTIFY statusChanged)
        Q_PROPERTY(OutgoingContactRequest *contactRequest READ contactRequest NOTIFY statusChanged)
        Q_PROPERTY(shims::ConversationModel *conversation READ conversation CONSTANT)

        friend class shims::ContactsManager;

    public:
        enum Status
        {
            Online,
            Offline,
            RequestPending,
            RequestRejected,
            Outdated
        };

        ContactUser(tego_context_t* context, ::ContactUser*);

        QString nickname() const;
        QString contactID() const;
        Status status() const;
        OutgoingContactRequest *contactRequest();
        shims::ConversationModel *conversation();

    public slots:
        void setNickname(const QString &nickname);
		
    signals:
        void nicknameChanged();
        void statusChanged();
        void contactDeleted(shims::ContactUser *user);

    private:
        tego_context_t* context;
        ::ContactUser* contactUser;
        shims::ConversationModel* conversationModel;

        friend class shims::ConversationModel;
    };
}