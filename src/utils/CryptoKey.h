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
