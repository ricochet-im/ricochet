// C headers

// os
#ifdef Q_OS_WIN
#include <wincrypt.h>
#endif

// standard library
#include <limits.h>

// openssl
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

// tego
#include <tego/tego.h>

// C++ headers
#ifdef __cplusplus

// standard library
#include <sstream>
#include <iomanip>
#include <cassert>
#include <type_traits>
#include <cstdint>

// Qt
#include <QAbstractListModel>
#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QClipboard>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QExplicitlySharedDataPointer>
#include <QFile>
#include <QFileInfo>
#include <QFlags>
#include <QGuiApplication>
#include <QHash>
#include <QHostAddress>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QLibraryInfo>
#include <QList>
#include <QLocale>
#include <QLockFile>
#include <QMap>
#include <QMessageAuthenticationCode>
#include <QMessageBox>
#include <QMetaType>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QObject>
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
#include <QSettings>
#include <QSharedData>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtDebug>
#include <QtEndian>
#include <QtGlobal>
#include <QTime>
#include <QTimer>
#ifdef Q_OS_MAC
#   include <QtMac>
#endif // Q_OS_MAC
#include <QtQml>
#include <QTranslator>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <QVector>

// tego
#include <tego/logger.hpp>
#include <tego/tego.hpp>

#endif // __cplusplus


