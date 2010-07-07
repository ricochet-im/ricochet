/* Torsion - http://github.com/special/torsion
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#ifndef CRYPTOKEY_H
#define CRYPTOKEY_H

#include <QString>
#include <QSharedData>
#include <QExplicitlySharedDataPointer>

class CryptoKey
{
public:
    CryptoKey();
    CryptoKey(const CryptoKey &other) : d(other.d) { }
    ~CryptoKey();

    static void test(const QString &file);

    bool loadFromData(const QByteArray &data, bool privateKey = false);
    bool loadFromFile(const QString &path, bool privateKey = false);
    void clear();

    bool isValid() const { return d.data() != 0; }
    bool isLoaded() const { return d.data() && d->key != 0; }
    bool isPrivate() const;

    QByteArray publicKeyDigest() const;
    QByteArray encodedPublicKey() const;
    QString torServiceID() const;

    /* Raw signatures; no digest */
    QByteArray signData(const QByteArray &data) const;
    bool verifySignature(const QByteArray &data, const QByteArray &signature) const;

private:
    struct Data : public QSharedData
    {
        typedef struct rsa_st RSA;
        RSA *key;

        Data(RSA *k = 0) : key(k) { }
        ~Data();
    };

    QExplicitlySharedDataPointer<Data> d;
};

#endif // CRYPTOKEY_H
