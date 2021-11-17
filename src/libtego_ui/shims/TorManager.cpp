#include "TorManager.h"

namespace shims
{
    TorManager* TorManager::torManager = nullptr;

    TorManager::TorManager(tego_context_t* context)
    : m_context(context)
    {
        // this ultimately controls whether the config dialog appears on launch
        if (this->configurationNeeded())
        {
            emit this->configurationNeededChanged();
        }
    }

    bool TorManager::configurationNeeded() const
    {
        logger::trace();

        tego_bool_t daemonConfigured = TEGO_FALSE;
        tego_context_get_tor_daemon_configured(
            m_context,
            &daemonConfigured,
            tego::throw_on_error());

        return (daemonConfigured == TEGO_FALSE);
    }

    QStringList TorManager::logMessages() const
    {
        const auto bufferSize = tego_context_get_tor_logs_size(
            m_context,
            tego::throw_on_error());
        auto buffer = std::make_unique<char[]>(bufferSize);

        // NOTE: it is possible for a new log entry to have been received in-between
        // getting the required buffer size and the data

        const auto written = tego_context_get_tor_logs(
            m_context,
            buffer.get(),
            bufferSize,
            tego::throw_on_error());

        Q_ASSERT(written < std::numeric_limits<int>::max());
        return QString::fromUtf8(buffer.get(), static_cast<int>(written)).split('\n');
    }

    QString TorManager::running() const
    {
        return this->m_running;
    }

    void TorManager::setRunning(const QString& running)
    {
        this->m_running = running;
        emit this->runningChanged();
    }

    bool TorManager::hasError() const
    {
        return !this->m_errorMessage.isEmpty();
    }

    QString TorManager::errorMessage() const
    {
        return this->m_errorMessage;
    }

    void TorManager::setErrorMessage(const QString& msg)
    {
        this->m_errorMessage = msg;
        emit this->errorChanged();
    }
}
