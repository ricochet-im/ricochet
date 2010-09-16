/* Torsion - http://torsionim.org/
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

#ifndef TORCONNTESTWIDGET_H
#define TORCONNTESTWIDGET_H

#include <QWidget>

namespace Tor
{
    class TorControlManager;
}

class QLabel;

namespace TorConfig
{

class TorConnTestWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(TorConnTestWidget)

public:
    explicit TorConnTestWidget(QWidget *parent = 0);

    void startTest(const QString &host, quint16 port, const QByteArray &authPassword);

    bool hasTestSucceeded() const { return m_state == 1; }
    bool hasTestFailed() const { return m_state == 0; }

    bool isTestRunning() const { return testManager != 0; }
    bool hasTestCompleted() const { return m_state >= 0; }

public slots:
    void clear();

signals:
    void testStarted();
    void testFinished(bool success);
    bool testSucceeded();
    bool testFailed();

    void stateChanged();

private slots:
    void doTestSuccess();
    void doTestFail();

    void torStatusChanged(int status);

private:
    QLabel *infoLabel;
    Tor::TorControlManager *testManager;
    qint8 m_state;
};

}

#endif // TORCONNTESTWIDGET_H
