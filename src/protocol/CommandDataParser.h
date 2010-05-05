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

#ifndef COMMANDDATAPARSER_H
#define COMMANDDATAPARSER_H

#include <QByteArray>

class QString;

class CommandDataParser
{
public:
    static const int maxCommandSize = 65535;

    CommandDataParser(QByteArray *data);
    CommandDataParser(const QByteArray *data);

    QByteArray *data() { return d; }
    void setData(QByteArray *data);
    void setData(const QByteArray *data);

    /* Read position; does not affect writes */
    int pos() const { return p; }
    void setPos(int pos);

    /* Indicates truncation or read-only for writes or missing data for reads */
    bool hasError() const { return error; }
    operator bool() const { return !hasError(); }
    bool operator!() const { return hasError(); }

    bool testSize(int i) const { return i < d->size(); }

    /* Input */
    CommandDataParser &operator<<(bool boolean);
    CommandDataParser &operator<<(qint8 byte);
    CommandDataParser &operator<<(quint8 byte);
    CommandDataParser &operator<<(qint16 value);
    CommandDataParser &operator<<(quint16 value);
    CommandDataParser &operator<<(qint32 value);
    CommandDataParser &operator<<(quint32 value);
    CommandDataParser &operator<<(qint64 value);
    CommandDataParser &operator<<(quint64 value);
    CommandDataParser &operator<<(const QString &string);

    CommandDataParser &writeFixedData(const QByteArray &data);
    CommandDataParser &writeVariableData(const QByteArray &data);

    /* Output */
    CommandDataParser &operator>>(bool &boolean);
    CommandDataParser &operator>>(qint8 &byte);
    CommandDataParser &operator>>(quint8 &byte);
    CommandDataParser &operator>>(qint16 &value);
    CommandDataParser &operator>>(quint16 &value);
    CommandDataParser &operator>>(qint32 &value);
    CommandDataParser &operator>>(quint32 &value);
    CommandDataParser &operator>>(qint64 &value);
    CommandDataParser &operator>>(quint64 &value);
    CommandDataParser &operator>>(QString &string);

    CommandDataParser &readFixedData(QByteArray *dest, int size);
    CommandDataParser &readVariableData(QByteArray *dest);

private:
    QByteArray *d;
    int p;
    bool writable, error;

    template<typename T> bool appendInt(T value);
    template<typename T> bool takeInt(T &value);
};

#endif // COMMANDDATAPARSER_H
