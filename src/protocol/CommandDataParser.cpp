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

#include "CommandDataParser.h"
#include <QtEndian>
#include <QString>

CommandDataParser::CommandDataParser(QByteArray *data)
    : d(data), p(0), writable(true), error(false)
{
}

CommandDataParser::CommandDataParser(const QByteArray *data)
    : d(const_cast<QByteArray*>(data)), p(0), writable(false), error(false)
{
}

void CommandDataParser::setData(QByteArray *data)
{
    d = data;
    p = 0;
    writable = true;
    error = false;
}

void CommandDataParser::setData(const QByteArray *data)
{
    d = const_cast<QByteArray*>(data);
    p = 0;
    writable = error = false;
}

void CommandDataParser::setPos(int pos)
{
    Q_ASSERT(pos >= 0 && pos < d->size());
    p = pos;
}

/* Helpers */
template<typename T> bool CommandDataParser::appendInt(T value)
{
    Q_ASSERT(writable);
    if (!writable || d->size() + int(sizeof(T)) > maxCommandSize)
    {
        error = true;
        return false;
    }

    value = qToBigEndian(value);
    d->append(reinterpret_cast<const char*>(&value), sizeof(T));

    return true;
}

template<> bool CommandDataParser::appendInt<quint8>(quint8 value)
{
    Q_ASSERT(writable);
    if (!writable || d->size() == maxCommandSize)
    {
        error = true;
        return false;
    }

    d->append(static_cast<char>(value));
    return true;
}

template<typename T> bool CommandDataParser::takeInt(T &value)
{
    if (p + int(sizeof(T)) > d->size())
    {
        error = true;
        return false;
    }

    value = qFromBigEndian<T>(reinterpret_cast<const uchar*>(d->constData()) + p);
    p += sizeof(T);

    return true;
}

template<> bool CommandDataParser::takeInt<quint8>(quint8 &value)
{
    if (p == d->size())
    {
        error = true;
        return false;
    }

    value = *((quint8*)(d->constData() + p));
    p++;
    return true;
}

/* Input - integer types */
CommandDataParser &CommandDataParser::operator<<(bool boolean)
{
    appendInt(static_cast<quint8>(boolean));
    return *this;
}

CommandDataParser &CommandDataParser::operator<<(qint8 value)
{
    appendInt(static_cast<quint8>(value));
    return *this;
}

CommandDataParser &CommandDataParser::operator<<(quint8 byte)
{
    appendInt(byte);
    return *this;
}

CommandDataParser &CommandDataParser::operator<<(qint16 value)
{
    appendInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator<<(quint16 value)
{
    appendInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator<<(qint32 value)
{
    appendInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator<<(quint32 value)
{
    appendInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator<<(qint64 value)
{
    appendInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator<<(quint64 value)
{
    appendInt(value);
    return *this;
}

/* Input */
CommandDataParser &CommandDataParser::operator<<(const QString &string)
{
    Q_ASSERT(writable);
    if (!writable || maxCommandSize - d->size() < 2)
    {
        error = true;
        return *this;
    }

    QByteArray encoded = string.toUtf8();
    if (encoded.size() > (maxCommandSize - d->size() - 2))
    {
        error = true;
        encoded.resize(maxCommandSize - d->size() - 2);
    }

    d->reserve(d->size() + encoded.size() + 2);
    appendInt((quint16)encoded.size());
    d->append(encoded);

    Q_ASSERT(d->size() <= maxCommandSize);

    return *this;
}

/* Output - integer types */
CommandDataParser &CommandDataParser::operator>>(bool &boolean)
{
    takeInt(reinterpret_cast<quint8&>(boolean));
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(qint8 &byte)
{
    takeInt(reinterpret_cast<quint8&>(byte));
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(quint8 &byte)
{
    takeInt(byte);
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(qint16 &value)
{
    takeInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(quint16 &value)
{
    takeInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(qint32 &value)
{
    takeInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(quint32 &value)
{
    takeInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(qint64 &value)
{
    takeInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(quint64 &value)
{
    takeInt(value);
    return *this;
}

CommandDataParser &CommandDataParser::operator>>(QString &string)
{
    quint16 length = 0;
    if (takeInt(length) && (p + length) <= d->size())
    {
        string = QString::fromUtf8(d->constData() + p, length);
        p += length;
    }
    else
        error = true;

    return *this;
}
