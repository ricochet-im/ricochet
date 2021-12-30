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

#include "CryptoKey.h"
#include "SecureRNG.h"
#include "Useful.h"
#include "utils/StringUtil.h"

bool CryptoKey::loadFromServiceId(const QByteArray& data)
{
    this->clear();
    // convert string to service id
    std::unique_ptr<tego_v3_onion_service_id_t> serviceId;
    tego_v3_onion_service_id_from_string(
        tego::out(serviceId),
        data.data(),
        static_cast<size_t>(data.size()),
        tego::throw_on_error());

    // extract public key from service id
    std::unique_ptr<tego_ed25519_public_key_t> publicKey;
    tego_ed25519_public_key_from_v3_onion_service_id(
        tego::out(publicKey),
        serviceId.get(),
        tego::throw_on_error());
    this->publicKey_ = std::move(publicKey);

    return true;
}

bool CryptoKey::loadFromKeyBlob(const QByteArray& keyBlob)
{
    this->clear();

    // convert keyblob to private key
    std::unique_ptr<tego_ed25519_private_key_t> privateKey;
    tego_ed25519_private_key_from_ed25519_keyblob(
        tego::out(privateKey),
        keyBlob.data(),
        static_cast<size_t>(keyBlob.size()),
        tego::throw_on_error());
    this->privateKey_ = std::move(privateKey);

    // claculate public key from private
    std::unique_ptr<tego_ed25519_public_key_t> publicKey;
    tego_ed25519_public_key_from_ed25519_private_key(
        tego::out(publicKey),
        this->privateKey_.get(),
        tego::throw_on_error());
    this->publicKey_ = std::move(publicKey);

    return true;
}

void CryptoKey::clear()
{
    privateKey_ = {};
    publicKey_ = {};
}

bool CryptoKey::isPrivate() const
{
    return privateKey_.get() != nullptr;
}

QByteArray CryptoKey::encodedKeyBlob() const
{
    // encode private key to KeyBlob
    char keyBlob[TEGO_ED25519_KEYBLOB_SIZE] = {0};
    tego_ed25519_keyblob_from_ed25519_private_key(keyBlob, sizeof(keyBlob), this->privateKey_.get(), tego::throw_on_error());

    QByteArray retval = {keyBlob, TEGO_ED25519_KEYBLOB_LENGTH};
    return retval;
}

QByteArray CryptoKey::torServiceID() const
{
    // convert public key to service id
    std::unique_ptr<tego_v3_onion_service_id_t> serviceId;
    tego_v3_onion_service_id_from_ed25519_public_key(
        tego::out(serviceId),
        this->publicKey_.get(),
        tego::throw_on_error());

    // service id to string
    char serviceIdString[TEGO_V3_ONION_SERVICE_ID_SIZE] = {0};
    tego_v3_onion_service_id_to_string(
        serviceId.get(),
        serviceIdString,
        sizeof(serviceIdString),
        tego::throw_on_error());

    QByteArray retval(serviceIdString);
    return retval;
}

QByteArray CryptoKey::signData(const QByteArray &msg) const
{
    // calculate signature
    std::unique_ptr<tego_ed25519_signature_t> signature;
    tego_message_ed25519_sign(
        reinterpret_cast<const uint8_t*>(msg.data()),
        static_cast<size_t>(msg.size()),
        this->privateKey_.get(),
        this->publicKey_.get(),
        tego::out(signature),
        tego::throw_on_error());

    // signature to byte blob
    QByteArray retval(TEGO_ED25519_SIGNATURE_SIZE, 0x00);
    tego_ed25519_signature_to_bytes(
        signature.get(),
        reinterpret_cast<uint8_t*>(retval.data()),
        static_cast<size_t>(retval.size()),
        tego::throw_on_error());

    return retval;
}

bool CryptoKey::verifyData(const QByteArray &msg, QByteArray signatureBytes) const
{
    // load signature from buffer
    std::unique_ptr<tego_ed25519_signature_t> signature;
    tego_ed25519_signature_from_bytes(
        tego::out(signature),
        reinterpret_cast<const uint8_t*>(signatureBytes.data()),
        static_cast<size_t>(signatureBytes.size()),
        tego::throw_on_error());

    // verify it against msg and our public key
    return tego_ed25519_signature_verify(
        signature.get(),
        reinterpret_cast<const uint8_t*>(msg.data()),
        static_cast<size_t>(msg.size()),
        this->publicKey_.get(),
        tego::throw_on_error());
}

/* Cryptographic hash of a password as expected by Tor's HashedControlPassword */
QByteArray torControlHashedPassword(const QByteArray &password)
{
    QByteArray salt = SecureRNG::random(8);
    if (salt.isNull())
        return QByteArray();

    int count = (16u + (96 & 15)) << ((96 >> 4) + 6);

    SHA_CTX hash;
    SHA1_Init(&hash);

    QByteArray tmp = salt + password;
    while (count)
    {
        int c = qMin(count, tmp.size());
        SHA1_Update(&hash, reinterpret_cast<const void*>(tmp.constData()), static_cast<size_t>(c));
        count -= c;
    }

    unsigned char md[20];
    SHA1_Final(md, &hash);

    /* 60 is the hex-encoded value of 96, which is a constant used by Tor's algorithm. */
    return QByteArray("16:") + salt.toHex().toUpper() + QByteArray("60") +
           QByteArray::fromRawData(reinterpret_cast<const char*>(md), 20).toHex().toUpper();
}
