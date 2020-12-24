/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

#ifndef CRYPTOKEY_H
#define CRYPTOKEY_H

class CryptoKey
{
public:
    // loads public key from service id
    bool loadFromServiceId(const QByteArray &data);
    // load private key from ed25519 KeyBlob format
    bool loadFromKeyBlob(const QByteArray& keyBlob);
    // load from Service Id

    void clear();

    bool isLoaded() const { return privateKey_ != nullptr || publicKey_ != nullptr; }
    bool isPrivate() const;

    // write to tor's 'KeyBlob' format
    QByteArray encodedKeyBlob() const;
    QByteArray torServiceID() const;

    // sign data with our private key
    QByteArray signData(const QByteArray &data) const;
    // verify data signature against public key
    bool verifyData(const QByteArray &data, QByteArray signature) const;

private:
    std::shared_ptr<tego_ed25519_private_key_t> privateKey_;
    std::shared_ptr<tego_ed25519_public_key_t> publicKey_;
};

QByteArray torControlHashedPassword(const QByteArray &password);

#endif // CRYPTOKEY_H
