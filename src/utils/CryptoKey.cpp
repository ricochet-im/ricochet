#include "CryptoKey.h"
#include <QtDebug>
#include <QFile>
#include <openssl/bio.h>
#include <openssl/pem.h>

CryptoKey::CryptoKey()
{

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
    RSA *key = PEM_read_bio_RSAPrivateKey(b, NULL, NULL, NULL);

    BIO_free(b);

    if (!key)
    {
        qDebug() << "Failed to parse private key from" << path;
        return false;
    }

    return true;
}
