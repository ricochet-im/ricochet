#ifndef CONVERSATIONMODEL_H
#define CONVERSATIONMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include "core/ContactUser.h"

class ConversationModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(ContactUser* contact READ contact WRITE setContact NOTIFY contactChanged)

public:
    enum {
        TimestampRole = Qt::UserRole,
        IsOutgoingRole
    };

    ConversationModel(QObject *parent = 0);

    ContactUser *contact() const { return m_contact; }
    void setContact(ContactUser *contact);

    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

public slots:
    void sendMessage(const QString &text);

signals:
    void contactChanged();

private slots:
    void receiveMessage(const ChatMessageData &message);

private:
    struct MessageData {
        QString text;
        QDateTime time;
        bool isOutgoing;
    };

    ContactUser *m_contact;
    QHash<int,QByteArray> roles;
    QList<MessageData> messages;
};

#endif

