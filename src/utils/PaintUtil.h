#ifndef PAINTUTIL_H
#define PAINTUTIL_H

class QPixmap;
class QSize;

#include <QStyle>

QPixmap customSelectionRect(const QSize &size, QStyle::State state);

#endif // PAINTUTIL_H
