/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
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

CommandDataParser &CommandDataParser::writeFixedData(const QByteArray &data)
{
    Q_ASSERT(writable);
    if (!writable || d->size() + data.size() > maxCommandSize)
    {
        error = true;
        return *this;
    }

    d->append(data);
    return *this;
}

CommandDataParser &CommandDataParser::writeVariableData(const QByteArray &data)
{
    Q_ASSERT(data.size() <= 65535);
    if (data.size() > 65535)
    {
        error = true;
        return *this;
    }

    *this << (quint16)data.size();
    writeFixedData(data);
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

CommandDataParser &CommandDataParser::readFixedData(QByteArray *dest, int size)
{
    if ((p + size) > d->size())
    {
        error = true;
        return *this;
    }

    *dest = d->mid(p, size);
    p += size;

    return *this;
}

CommandDataParser &CommandDataParser::readVariableData(QByteArray *dest)
{
    quint16 length = 0;

    if (!takeInt(length))
        error = true;
    else
        readFixedData(dest, length);

    return *this;
}
