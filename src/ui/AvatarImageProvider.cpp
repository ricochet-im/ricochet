#include "AvatarImageProvider.h"
#include "core/IdentityManager.h"

AvatarImageProvider::AvatarImageProvider()
    : QDeclarativeImageProvider(Pixmap)
{
}

QPixmap AvatarImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QPixmap re;
    QStringList sections = id.split(QLatin1Char('/'));
    if (sections.size() < 2)
        return re;

    bool ok = false;
    int identityId = sections[0].toInt(&ok);
    UserIdentity *identity = identityManager->lookupUniqueID(identityId);
    if (!ok || !identity)
        return re;

    if (sections[1] == QLatin1String("identity"))
    {
        QByteArray data = identity->readSetting("avatar").toByteArray();
        re.loadFromData(data);
    }
    else if (sections[1] == QLatin1String("contact") && sections.size() >= 3)
    {
        int contactId = sections[2].toInt(&ok);
        ContactUser *user = identity->contacts.lookupUniqueID(contactId);
        if (!ok || !user)
            return re;

        QByteArray data = user->readSetting("avatar").toByteArray();
        re.loadFromData(data);
    }

    if (!re.isNull())
    {
        *size = re.size();
        if (requestedSize.isValid())
            re = re.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return re;
}
