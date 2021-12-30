#include <QtGlobal>

// C headers

// os
#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
// workaround because protobuffer defines a GetMessage function
#undef GetMessage
#endif

// standard library
#include <stddef.h>

// openssl
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

// tor
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#   include <orconfig.h>
#   ifdef HAVE_EVP_SHA3_256
#       define OPENSSL_HAS_SHA3
#   endif // HAVE_EVP_SHA3_256
#   define ALL_BUGS_ARE_FATAL
#   include <src/lib/log/util_bug.h>
#   include <ext/ed25519/donna/ed25519_donna_tor.h>
#   include <src/lib/defs/x25519_sizes.h>
#   include <src/lib/encoding/binascii.h>
#   include <src/lib/crypt_ops/crypto_digest.h>
#ifdef __cplusplus
}

// include our public header
#include <tego/tego.hpp>

#endif // __cplusplus

// C++ headers
#ifdef __cplusplus

// standard library
#include <string_view>
#include <cstdio>
#include <stdexcept>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <optional>
#include <tuple>
#include <type_traits>
#include <chrono>

// fmt
#include <fmt/format.h>
#include <fmt/ostream.h>

// Qt
#include <QString>
#include <QByteArray>

// libtego_ui Qt
#include <QAbstractListModel>
#include <QBuffer>
#include <QClipboard>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QExplicitlySharedDataPointer>
#include <QFileInfo>
#include <QFlags>
#include <QGuiApplication>
#include <QHash>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QMessageAuthenticationCode>
#include <QMetaType>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QPair>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlNetworkAccessManagerFactory>
#include <QQueue>
#include <QQuickItem>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSaveFile>
#include <QScopedPointer>
#include <QScreen>
#include <QSet>
#include <QSharedData>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtDebug>
#include <QtEndian>
#include <QtGlobal>
#include <QTime>
#include <QTimer>
#include <QtQml>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <QVector>

namespace tego
{
    template<typename T, size_t N>
    constexpr size_t countof(T (&)[N])
    {
        return N;
    }
}

#endif //__cplusplus#i
