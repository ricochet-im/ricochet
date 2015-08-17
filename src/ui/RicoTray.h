#ifndef RICOTRAY_HPP
#define RICOTRAY_HPP
#include "MainWindow.h"
#include <QSystemTrayIcon>
#include "core/ConversationModel.h"

class RicoTray : public QSystemTrayIcon
{
    Q_OBJECT
    Q_PROPERTY(bool hidden READ isHidden WRITE setHidden NOTIFY hiddenChanged)
    Q_PROPERTY(bool unread READ isUnread WRITE setUnread NOTIFY unreadChanged)
public:
    RicoTray();

signals:
    void iconTriggered();
    void quitTriggered();
    void preferencesTriggered();
    void addContactTriggered();
    void hiddenChanged(bool);
    void unreadChanged(bool);

public slots:
    bool isHidden() const;
    void setHidden(bool);

    bool isUnread() const;
    void setUnread(bool);

    void quit();
    void preferences();
    void addContact();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QIcon std_icon;
    QIcon unread_icon;
    bool unread;
};

#endif // RICOTRAY_HPP
