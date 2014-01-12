#ifndef AVATARIMAGEPROVIDER_H
#define AVATARIMAGEPROVIDER_H

#include <QQuickImageProvider>

class AvatarImageProvider : public QQuickImageProvider
{
public:
    AvatarImageProvider();

    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // AVATARIMAGEPROVIDER_H
