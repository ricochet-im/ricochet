/* Torsion - http://github.com/special/torsion
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "BundledTorManager.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

BundledTorManager *BundledTorManager::m_instance = 0;

BundledTorManager::BundledTorManager()
{
    Q_ASSERT(!m_instance);
    m_instance = this;

    process.setProcessChannelMode(QProcess::MergedChannels);
    process.setReadChannel(QProcess::StandardOutput);

    start();
}

BundledTorManager *BundledTorManager::instance()
{
    if (!m_instance)
        m_instance = new BundledTorManager;

    return m_instance;
}

static QString torDirectory()
{
    return qApp->applicationDirPath() + QLatin1String("/Tor/");
}

static QString torExecutable()
{
#ifdef Q_OS_WIN
    return torDirectory() + QLatin1String("tor.exe");
#else
    return torDirectory() + QLatin1String("tor");
#endif
}

bool BundledTorManager::isAvailable()
{
    return QFile::exists(torExecutable());
}

void BundledTorManager::start()
{
    if (isRunning())
        return;

    qDebug() << "Launching bundled Tor from" << torExecutable();

    QString dir = torDirectory();
    QStringList args;

    args << QLatin1String("-f") << dir;
    args << QLatin1String("--DataDirectory") << dir;
    // ControlPort
    // HashedControlPassword

    process.setWorkingDirectory(dir);
    process.start(torExecutable(), args, QIODevice::ReadOnly);
}
