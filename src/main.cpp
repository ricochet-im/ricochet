/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#include "ui/MainWindow.h"
#include "core/ContactsManager.h"
#include "tor/TorControlManager.h"
#include "tor/HiddenService.h"
#include "ui/torconfig/TorConfigWizard.h"
#include "protocol/IncomingSocket.h"
#include <QApplication>
#include <QSettings>
#include <QTime>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>
#include <openssl/crypto.h>

QSettings *config = 0;

static IncomingSocket *incomingSocket = 0;

static void initSettings();
static void initTranslation();
static void initIncomingSocket();
static bool connectTorControl();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationVersion(QString("1.0.0"));

    QDir::setCurrent(a.applicationDirPath());

    initSettings();
    initTranslation();

    /* Seed RNG */
    {
        QTime now = QTime::currentTime();
        qsrand(unsigned(now.second()) * now.msec() * unsigned(a.applicationPid()));
    }

    /* Initialize OpenSSL's allocator */
    CRYPTO_malloc_init();

    /* Contacts */
    contactsManager = new ContactsManager;

    /* Incoming socket */
    initIncomingSocket();

    /* Tor control manager; this may enter into the TorConfigWizard. */
    if (!connectTorControl())
        return 0;

    QObject::connect(torManager, SIGNAL(socksReady()), contactsManager, SLOT(connectToAll()));

    /* Window */
    MainWindow w;
    w.show();

    return a.exec();
}

static void initSettings()
{
    /* Defaults */
    qApp->setOrganizationName(QString("TorIM"));
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, qApp->applicationDirPath());

    /* Commandline */
    QStringList args = qApp->arguments();
    if (args.size() > 1)
        config = new QSettings(args[1], QSettings::IniFormat);
    else
        config = new QSettings;
}

static void initTranslation()
{
    QTranslator *translator = new QTranslator;

    bool ok = false;
    QString appPath = qApp->applicationDirPath();

    /* First, try to load the user's configured language */
    QString configLang = config->value("core/language").toString();
    if (!configLang.isEmpty())
    {
        /* Look in the application directory */
        ok = translator->load(QString("torim.") + configLang, appPath, QString("_"));
        /* Look in the resources */
        if (!ok)
            ok = translator->load(QString("torim.") + configLang, QString(":/lang/"), QString("_"));
    }

    /* Next, try to load the system locale language, and allow it to fall back to the english default */
    if (!ok)
    {
        QString locale = QLocale::system().name();
        ok = translator->load(QString("torim.") + configLang, appPath);
        if (!ok)
            ok = translator->load(QString("torim.") + configLang, QString(":/lang/"));
    }

    if (ok)
        qApp->installTranslator(translator);
}

static void initIncomingSocket()
{
    QHostAddress address(config->value("core/listenIp", QString("127.0.0.1")).toString());
    quint16 port = (quint16)config->value("core/listenPort", 0).toUInt();

    incomingSocket = new IncomingSocket;
    if (!incomingSocket->listen(address, port))
        qFatal("Failed to open incoming socket: %s", qPrintable(incomingSocket->errorString()));
}

static bool connectTorControl()
{
    QHostAddress address(config->value("tor/controlIp").value<QString>());
    quint16 port = (quint16)config->value("tor/controlPort", 0).toUInt();

    if (address.isNull() || !port)
    {
        TorConfigWizard wizard;
        int re = wizard.exec();
        if (re != QDialog::Accepted)
            return false;

        address = config->value("tor/controlIp").value<QString>();
        port = (quint16)config->value("tor/controlPort", 0).toUInt();

        if (address.isNull() || !port)
        {
            QMessageBox::critical(0, wizard.tr("TorIM - Error"),
                wizard.tr("The Tor configuration wizard did not complete successfully. Please restart the "
                "application and try again."));
            return false;
        }
    }

    torManager = new Tor::TorControlManager;

    /* Authentication */
    torManager->setAuthPassword(config->value("tor/authPassword").toByteArray());

    /* Hidden service */
    QString serviceDir = config->value("core/serviceDirectory", QString("data")).toString();

    Tor::HiddenService *service = new Tor::HiddenService(serviceDir);
    service->addTarget(13535, incomingSocket->serverAddress(), incomingSocket->serverPort());

    torManager->addHiddenService(service);

    /* Connect */
    torManager->connect(address, port);
    return true;
}
