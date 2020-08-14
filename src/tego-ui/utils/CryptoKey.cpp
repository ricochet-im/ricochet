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

    try
    {
        // convert string to service id
        tego_v3_onion_service_id_t* pServiceId = nullptr;
        tego_v3_onion_service_id_from_string(
            &pServiceId,
            data.data(),
            data.size(),
            tego::throw_on_error());
        std::unique_ptr<tego_v3_onion_service_id_t> serviceId(pServiceId);

        // extract public key from service id
        tego_ed25519_public_key_t* pPublicKey = nullptr;
        tego_ed25519_public_key_from_v3_onion_service_id(
            &pPublicKey,
            serviceId.get(),
            tego::throw_on_error());
        this->publicKey_.reset(pPublicKey, std::default_delete<tego_ed25519_public_key_t>());

        return true;
    }
    catch(std::exception& ex)
    {
        logger::println("Caught Exception: {}", ex.what());
    }
    return false;
}

bool CryptoKey::loadFromKeyBlob(const QByteArray& keyBlob)
{
    this->clear();
    logger::trace();
    try
    {
        logger::println("Original KeyBlob:{}", keyBlob);

        // convert keyblob to private key
        tego_ed25519_private_key_t* pPrivateKey = nullptr;
        tego_ed25519_private_key_from_ed25519_keyblob(
            &pPrivateKey,
            keyBlob.data(),
            keyBlob.size(),
            tego::throw_on_error());
        this->privateKey_.reset(pPrivateKey, std::default_delete<tego_ed25519_private_key_t>());

        // claculate public key from private
        tego_ed25519_public_key_t* pPublicKey = nullptr;
        tego_ed25519_public_key_from_ed25519_private_key(
            &pPublicKey,
            this->privateKey_.get(),
            tego::throw_on_error());
        this->publicKey_.reset(pPublicKey, std::default_delete<tego_ed25519_public_key_t>());

        return true;
    }
    catch(std::exception& ex)
    {
        logger::println("Caught Exception: {}", ex.what());
    }
    return false;
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
    logger::trace();
    Q_ASSERT(this->privateKey_.get() != nullptr);

    char keyBlob[TEGO_ED25519_KEYBLOB_SIZE] = {0};
    tego_ed25519_keyblob_from_ed25519_private_key(keyBlob, sizeof(keyBlob), this->privateKey_.get(), tego::throw_on_error());

    logger::trace();
    QByteArray retval = {keyBlob, TEGO_ED25519_KEYBLOB_SIZE};
    logger::println("Encoded KeyBlob: {}", retval);
    return retval;
}

QString CryptoKey::torServiceID() const
{
    logger::trace();
    if (!this->publicKey_)
    {
        logger::println("Service Id: \"\"");
        return QString();
    }

    tego_v3_onion_service_id_t* pServiceId = nullptr;
    tego_v3_onion_service_id_from_ed25519_public_key(
        &pServiceId,
        this->publicKey_.get(),
        tego::throw_on_error());
    std::unique_ptr<tego_v3_onion_service_id_t> serviceId(pServiceId);

    char serviceIdString[TEGO_V3_ONION_SERVICE_ID_SIZE] = {0};
    tego_v3_onion_service_id_to_string(
        serviceId.get(),
        serviceIdString,
        sizeof(serviceIdString),
        tego::throw_on_error());

    QString retval = {serviceIdString};
    logger::println("Service Id: {}", retval);
    return retval;
}

QByteArray CryptoKey::signData(const QByteArray &msg) const
{
    logger::trace();
    if (!isPrivate())
        return {};

    tego_ed25519_signature_t* pSignature = nullptr;
    tego_message_ed25519_sign(
        reinterpret_cast<const uint8_t*>(msg.data()),
        msg.size(),
        this->privateKey_.get(),
        this->publicKey_.get(),
        &pSignature,
        tego::throw_on_error());
    std::unique_ptr<tego_ed25519_signature_t> signature(pSignature);

    QByteArray retval(TEGO_ED25519_SIGNATURE_SIZE, 0x00);
    tego_ed25519_signature_to_data(
        signature.get(),
        reinterpret_cast<uint8_t*>(retval.data()),
        retval.size(),
        tego::throw_on_error());

    logger::println("Signature: {}", retval);
    return retval;
}

bool CryptoKey::verifyData(const QByteArray &msg, QByteArray signatureBytes) const
{
    logger::trace();
    tego_ed25519_signature_t* pSignature = nullptr;
    tego_ed25519_signature_from_data(
        &pSignature,
        reinterpret_cast<const uint8_t*>(signatureBytes.data()),
        signatureBytes.size(),
        tego::throw_on_error());
    std::unique_ptr<tego_ed25519_signature_t> signature(pSignature);

    return tego_ed25519_signature_verify(
        signature.get(),
        reinterpret_cast<const uint8_t*>(msg.data()),
        msg.size(),
        this->publicKey_.get(),
        tego::throw_on_error());
}

/* Cryptographic hash of a password as expected by Tor's HashedControlPassword */
QByteArray torControlHashedPassword(const QByteArray &password)
{
    QByteArray salt = SecureRNG::random(8);
    if (salt.isNull())
        return QByteArray();

    int count = ((quint32)16 + (96 & 15)) << ((96 >> 4) + 6);

    SHA_CTX hash;
    SHA1_Init(&hash);

    QByteArray tmp = salt + password;
    while (count)
    {
        int c = qMin(count, tmp.size());
        SHA1_Update(&hash, reinterpret_cast<const void*>(tmp.constData()), c);
        count -= c;
    }

    unsigned char md[20];
    SHA1_Final(md, &hash);

    /* 60 is the hex-encoded value of 96, which is a constant used by Tor's algorithm. */
    return QByteArray("16:") + salt.toHex().toUpper() + QByteArray("60") +
           QByteArray::fromRawData(reinterpret_cast<const char*>(md), 20).toHex().toUpper();
}
