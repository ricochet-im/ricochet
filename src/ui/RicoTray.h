#ifndef RICOTRAY_HPP
#define RICOTRAY_HPP
#include "MainWindow.h"
#include <QSystemTrayIcon>

class RicoTray : public QSystemTrayIcon
{
    Q_OBJECT
    Q_PROPERTY(bool hidden READ isHidden WRITE setHidden NOTIFY hiddenChanged)
public:
    RicoTray();

signals:
    void iconTriggered();
    void quitTriggered();
    void preferencesTriggered();
    void addContactTriggered();
    void hiddenChanged(bool);

public slots:
    bool isHidden() const;
    void setHidden(bool);

    void quit();
    void preferences();
    void addContact();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // RICOTRAY_HPP
