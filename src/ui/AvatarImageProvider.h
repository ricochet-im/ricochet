#ifndef AVATARIMAGEPROVIDER_H
#define AVATARIMAGEPROVIDER_H

#include <QDeclarativeImageProvider>

class AvatarImageProvider : public QDeclarativeImageProvider
{
public:
    AvatarImageProvider();

    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif // AVATARIMAGEPROVIDER_H
