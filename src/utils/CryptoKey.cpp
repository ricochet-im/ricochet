#include "CryptoKey.h"
#include <QtDebug>
#include <QFile>
#include <openssl/bio.h>
#include <openssl/pem.h>

CryptoKey::CryptoKey()
    : key(0)
{
}

CryptoKey::~CryptoKey()
{
    clear();
}

void CryptoKey::clear()
{
    if (key)
    {
        RSA_free(key);
        key = 0;
    }
}

bool CryptoKey::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open private key from" << path << "-" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    BIO *b = BIO_new_mem_buf((void*)data.constData(), -1);
    key = PEM_read_bio_RSAPrivateKey(b, NULL, NULL, NULL);

    BIO_free(b);

    if (!key)
    {
        qDebug() << "Failed to parse private key from" << path;
        return false;
    }

    return true;
}

QByteArray CryptoKey::encodedPublicKey()
{
    if (!isLoaded())
        return QByteArray();

    BIO *b = BIO_new(BIO_s_mem());

    if (!PEM_write_bio_RSAPublicKey(b, key))
    {
        qDebug() << "Failed to encode public key";
        BIO_free(b);
        return QByteArray();
    }

    BUF_MEM *buf;
    BIO_get_mem_ptr(b, &buf);

    /* Close BIO, but don't free buf. */
    (void)BIO_set_close(b, BIO_NOCLOSE);
    BIO_free(b);

    QByteArray re((const char *)buf->data, (int)buf->length);
    BUF_MEM_free(buf);

    return re;
}

void CryptoKey::test(const QString &file)
{
    CryptoKey key;

    bool ok = key.loadFromFile(file);
    Q_ASSERT(ok);
    Q_ASSERT(key.isLoaded());

    QByteArray pubkey = key.encodedPublicKey();
    Q_ASSERT(!pubkey.isEmpty());

    qDebug() << "(crypto test) Public key:" << pubkey;
}
