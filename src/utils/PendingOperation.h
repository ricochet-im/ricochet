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

#ifndef PENDINGOPERATION_H
#define PENDINGOPERATION_H

#include <QObject>

/* Represents an asynchronous operation for reporting status
 *
 * This class is used for asynchronous operations that report a
 * status and errors when finished, particularly for exposing them
 * to QML.
 *
 * Subclass PendingOperation to implement your operation's logic.
 * You also need to handle the object's lifetime, for example by
 * calling deleteLater() when finished() is emitted.
 *
 * PendingOperation will emit finished() and one of success() or
 * error() when completed.
 */
class PendingOperation : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isFinished READ isFinished NOTIFY finished FINAL)
    Q_PROPERTY(bool isSuccess READ isSuccess NOTIFY success FINAL)
    Q_PROPERTY(bool isError READ isError NOTIFY error FINAL)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY finished FINAL)

public:
    PendingOperation(QObject *parent = 0);

    bool isFinished() const;
    bool isSuccess() const;
    bool isError() const;
    QString errorMessage() const;

signals:
    // Always emitted once when finished, regardless of status
    void finished();

    // One of error() or success() is emitted once
    void error(const QString &errorMessage);
    void success();

protected slots:
    void finishWithError(const QString &errorMessage);
    void finishWithSuccess();

private:
    bool m_finished;
    QString m_errorMessage;
};

Q_DECLARE_METATYPE(PendingOperation*)

#endif
