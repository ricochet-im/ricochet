#ifndef DATEUTIL_H
#define DATEUTIL_H

#include <QString>

class QDateTime;

QString timeDifferenceString(const QDateTime &from, const QDateTime &to);

#endif // DATEUTIL_H
