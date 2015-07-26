#ifndef RICOTRAY_HPP
#define RICOTRAY_HPP
#include "MainWindow.h"
#include <QSystemTrayIcon>

class RicoTray : public QSystemTrayIcon
{
public:
    RicoTray(MainWindow *p_main = nullptr);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    MainWindow* main;
};

#endif // RICOTRAY_HPP
