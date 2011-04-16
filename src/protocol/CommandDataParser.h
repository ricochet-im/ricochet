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
