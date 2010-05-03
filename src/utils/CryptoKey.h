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

#ifndef CRYPTOKEY_H
#define CRYPTOKEY_H

#include <QString>

class CryptoKey
{
public:
    CryptoKey();
    ~CryptoKey();

    static void test(const QString &file);

    bool loadFromFile(const QString &path);
    void clear();

    bool isLoaded() const { return key != 0; }
    bool isPrivate() const;

    QByteArray publicKeyDigest() const;
    QByteArray encodedPublicKey() const;
    QString torServiceID() const;

    /* Raw signatures; no digest */
    QByteArray signData(const QByteArray &data) const;
    bool verifySignature(const QByteArray &data, const QByteArray &signature) const;

private:
    typedef struct rsa_st RSA;

    RSA *key;
};

#endif // CRYPTOKEY_H
