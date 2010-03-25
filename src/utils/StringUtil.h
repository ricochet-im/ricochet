#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <QByteArray>

QByteArray quotedString(const QByteArray &string);
QByteArray unquotedString(const QByteArray &string, int *pos = 0);

#endif // STRINGUTIL_H
