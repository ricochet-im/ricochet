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
    void hiddenChanged(bool);

public slots:
    bool isHidden() const;
    void setHidden(bool);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    bool hidden;
};

#endif // RICOTRAY_HPP
