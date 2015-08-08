#ifndef RICOTRAY_HPP
#define RICOTRAY_HPP
#include "MainWindow.h"
#include <QSystemTrayIcon>

class RicoTray : public QSystemTrayIcon
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
public:
    RicoTray();

signals:
    void iconTriggered();
    void enabledChanged(bool);

public slots:
    bool isEnabled() const;
    void setEnabled(bool);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    bool enabled;
};

#endif // RICOTRAY_HPP
