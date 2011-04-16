/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
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
