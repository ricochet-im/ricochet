#pragma once

namespace shims
{
    class TorManager : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool configurationNeeded READ configurationNeeded NOTIFY configurationNeededChanged)
        Q_PROPERTY(QStringList logMessages READ logMessages CONSTANT)
        Q_PROPERTY(QString running READ running NOTIFY runningChanged)
        Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
        Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)

    public:
        TorManager(tego_context_t*);
        static TorManager* torManager;

        bool configurationNeeded() const;
        QStringList logMessages() const;
        QString running() const;
        void setRunning(const QString& running);
        bool hasError() const;
        QString errorMessage() const;
        void setErrorMessage(const QString& message);
    signals:
        void configurationNeededChanged();
        void logMessage(const QString &message);
        void runningChanged();
        void errorChanged();

    private:
        tego_context_t* m_context;
        QString m_errorMessage;
        QString m_running;
    };
}