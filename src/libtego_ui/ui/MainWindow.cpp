/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ui/MainWindow.h"
#include "ui/Clipboard.h"
#include "ui/ContactsModel.h"
#include "ui/LanguagesModel.h"

#include "utils/Settings.h"
#include "utils/Useful.h"


// shim replacements
#include "shims/TorControl.h"
#include "shims/TorManager.h"
#include "shims/UserIdentity.h"
#include "shims/ContactsManager.h"
#include "shims/ContactUser.h"
#include "shims/ConversationModel.h"
#include "shims/OutgoingContactRequest.h"
#include "shims/ContactIDValidator.h"
#include "shims/IncomingContactRequest.h"

MainWindow *uiMain = 0;

/* Through the QQmlNetworkAccessManagerFactory below, all network requests
 * created via QML will be passed to this object; including, for example,
 * <img> tags parsed in rich Text items.
 *
 * Ricochet's UI does not directly cause network requests for any reason. These
 * are always a potentially deanonymizing bug. This object will block them,
 * and assert if appropriate.
 */
class BlockedNetworkAccessManager : public QNetworkAccessManager
{
public:
    BlockedNetworkAccessManager(QObject *parent)
        : QNetworkAccessManager(parent)
    {
        /* This will cause any network request to fail.
         * This should be redundant, because createRequest below also
         * blackholes every request (and crashes for assert builds). */
        setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, QLatin1String("0.0.0.0"), 0));
    }

protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0)
    {
        TEGO_BUG() << "QML attempted to load a network resource from" << req.url() << " - this is potentially an input sanitization flaw.";
        return QNetworkAccessManager::createRequest(op, QNetworkRequest(), outgoingData);
    }
};

class NetworkAccessBlockingFactory : public QQmlNetworkAccessManagerFactory
{
public:
    virtual QNetworkAccessManager *create(QObject *parent)
    {
        return new BlockedNetworkAccessManager(parent);
    }
};

MainWindow::MainWindow(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(!uiMain);
    uiMain = this;

    qml = new QQmlApplicationEngine(this);
    qml->setNetworkAccessManagerFactory(new NetworkAccessBlockingFactory);

    qmlRegisterUncreatableType<shims::ContactUser>("im.ricochet", 1, 0, "ContactUser", QString());
    qmlRegisterUncreatableType<shims::UserIdentity>("im.ricochet", 1, 0, "UserIdentity", QString());
    qmlRegisterUncreatableType<shims::ContactsManager>("im.ricochet", 1, 0, "ContactsManager", QString());
    qmlRegisterUncreatableType<shims::IncomingContactRequest>("im.ricochet", 1, 0, "IncomingContactRequest", QString());
    qmlRegisterUncreatableType<shims::OutgoingContactRequest>("im.ricochet", 1, 0, "OutgoingContactRequest", QString());
    qmlRegisterUncreatableType<shims::TorControl>("im.ricochet", 1, 0, "TorControl", QString());
    qmlRegisterType<shims::ConversationModel>("im.ricochet", 1, 0, "ConversationModel");
    qmlRegisterType<::ContactsModel>("im.ricochet", 1, 0, "ContactsModel");
    qmlRegisterType<shims::ContactIDValidator>("im.ricochet", 1, 0, "ContactIDValidator");
    qmlRegisterType<::SettingsObject>("im.ricochet", 1, 0, "Settings");
    qmlRegisterSingletonType<::Clipboard>("im.ricochet", 1, 0, "Clipboard", &Clipboard::singleton_provider);
    qmlRegisterType<::LanguagesModel>("im.ricochet", 1, 0, "LanguagesModel");
}

MainWindow::~MainWindow()
{
}

bool MainWindow::showUI()
{
    qml->rootContext()->setContextProperty(QLatin1String("userIdentity"), shims::UserIdentity::userIdentity);
    qml->rootContext()->setContextProperty(QLatin1String("torControl"), shims::TorControl::torControl);
    qml->rootContext()->setContextProperty(QLatin1String("torInstance"), shims::TorManager::torManager);
    qml->rootContext()->setContextProperty(QLatin1String("uiMain"), this);

    qml->load(QUrl(QLatin1String("qrc:/ui/main.qml")));

    if (qml->rootObjects().isEmpty()) {
        // Assume this is only applicable to technical users; not worth translating or simplifying.
        QMessageBox::critical(0, QStringLiteral("Ricochet"),
            QStringLiteral("An error occurred while loading the Ricochet UI.\n\n"
                           "You might be missing plugins or dependency packages."));
        qCritical() << "Failed to load UI. Exiting.";
        return false;
    }

    return true;
}

QString MainWindow::version() const
{
    const static auto retval = qApp->applicationVersion();
    return retval;
}

QString MainWindow::accessibleVersion() const
{
    const static auto retval = [this]() -> QString
    {
        auto version = this->version();
        return version.replace('.', QString(" %1 ").arg(tr("Version Seperator")));
    }();

    return retval;
}

QString MainWindow::aboutText() const
{
    QFile file(QStringLiteral(":/text/LICENSE"));
    file.open(QIODevice::ReadOnly);
    QString text = QString::fromUtf8(file.readAll());
    return text;
}

QVariantMap MainWindow::screens() const
{
    QVariantMap mapScreenSizes;
    foreach (QScreen *screen, QGuiApplication::screens()) {
        QVariantMap screenObj;
        screenObj.insert(QString::fromUtf8("width"), screen->availableSize().width());
        screenObj.insert(QString::fromUtf8("height"), screen->availableSize().height());
        screenObj.insert(QString::fromUtf8("left"), screen->geometry().left());
        screenObj.insert(QString::fromUtf8("top"), screen->geometry().top());
        mapScreenSizes.insert(screen->name(), screenObj);
    }
    return mapScreenSizes;
}

/* QMessageBox implementation for Qt <5.2 */
bool MainWindow::showRemoveContactDialog(shims::ContactUser *user)
{
    if (!user)
        return false;
    QMessageBox::StandardButton btn = QMessageBox::question(0,
        tr("Remove %1").arg(user->getNickname()),
        tr("Do you want to permanently remove %1?").arg(user->getNickname()));
    return btn == QMessageBox::Yes;
}

QQuickWindow *MainWindow::findParentWindow(QQuickItem *item)
{
    Q_ASSERT(item);
    return item ? item->window() : 0;
}
