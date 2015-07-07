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

#include "LanguagesModel.h"
#include <QDir>
#include <QLocale>
#include <QVariant>

LanguagesModel::LanguagesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    // create the list of languages based on present translation files in ":/lang" folder
    QDir languagesFolder(QStringLiteral(":/lang"));
    languages.append(LanguageEntry(tr("System default"), QString()));
    foreach (const QString& translationFile, languagesFolder.entryList()) {
        QString localeID = translationFile;
        localeID.remove(QLatin1String("ricochet_")).remove(QLatin1String(".qm"));
        QString nativeName = QLocale(localeID).nativeLanguageName();
        languages.append(LanguageEntry(nativeName, localeID));
    }
}

int LanguagesModel::rowCount(const QModelIndex &) const
{
    return languages.length();
}

QVariant LanguagesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= 0 && index.row() < languages.length()) {
        switch( role ) {
            case NameRole:
                return languages[index.row()].nativeName;
            case LocaleIDRole:
                return languages[index.row()].localeID;
            default:
                return QVariant();
        }
    } else {
        return QVariant();
    }
}

QHash<int, QByteArray> LanguagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "nativeName";
    roles[LocaleIDRole] = "localeID";
    return roles;
}

