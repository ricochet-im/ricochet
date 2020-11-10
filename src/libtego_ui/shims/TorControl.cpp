#include "TorControl.h"

namespace shims
{
    TorControl* TorControl::torControl = nullptr;

    TorControl::TorControl(tego_context_t* context)
    : context(context)
    { }

    QObject* TorControl::setConfiguration(const QVariantMap &options)
    {
        logger::trace();
        Q_ASSERT(this->m_setConfigurationCommand == nullptr);

        std::unique_ptr<tego_tor_daemon_config_t> daemonConfig;

        // create command shim yuck
        auto setConfigurationCommand = new TorControlCommand();
        QQmlEngine::setObjectOwnership(setConfigurationCommand, QQmlEngine::CppOwnership);

        this->m_setConfigurationCommand = setConfigurationCommand;

        tego_tor_daemon_config_initialize(
            tego::out(daemonConfig),
            tego::throw_on_error());

        // TODO: fill out daemon config

        tego_context_update_tor_daemon_config(
            context,
            daemonConfig.get(),
            tego::throw_on_error());

        return this->m_setConfigurationCommand;
    }

    // for now we just assume we always have ownership,
    // as we have no way in config to setup usage of
    // an existing tor process
    bool TorControl::hasOwnership() const
    {
        logger::trace();
        return true;
    }

    void TorControl::saveConfiguration()
    {
        logger::trace();
        tego_context_save_tor_daemon_config(
            context,
            tego::throw_on_error());
    }

    QString TorControl::torVersion() const
    {
        logger::trace();
        return tego_context_get_tor_version_string(
            context,
            tego::throw_on_error());
    }

    TorControl::Status TorControl::status() const
    {
        tego_tor_control_status_t status;
        tego_context_get_tor_control_status(
            context,
            &status,
            tego::throw_on_error());

        logger::trace();
        return static_cast<TorControl::Status>(status);
    }

    TorControl::TorStatus TorControl::torStatus() const
    {
        tego_tor_daemon_status_t status;
        tego_context_get_tor_daemon_status(
            context,
            &status,
            tego::throw_on_error());

        logger::trace();
        return static_cast<TorControl::TorStatus>(status);
    }

    QVariantMap TorControl::bootstrapStatus() const
    {
        QVariantMap retval;
        try
        {
            int32_t progress;
            tego_tor_bootstrap_tag_t tag;
            tego_context_get_tor_bootstrap_status(
                context,
                &progress,
                &tag,
                tego::throw_on_error());

            auto tagSummary = tego_tor_bootstrap_tag_to_summary(
                    tag,
                    tego::throw_on_error());

            retval["severity"] = "mock severity";
            retval["progress"] = progress;
            retval["tag"] = (tag == tego_tor_bootstrap_tag_done) ? "done" : "mock_tag";
            retval["summary"] = QString(tagSummary);
        }
        catch(const std::exception& ex)
        {
            logger::println("Exception thrown : {}", ex.what());
        }
        return retval;
    }

    QString TorControl::errorMessage() const
    {
        return m_errorMessage;
    }

    void TorControl::setStatus(Status status)
    {
        auto oldStatus = m_status;
        if (oldStatus == status) return;

        m_status = status;
        emit this->statusChanged(
            static_cast<int>(status),
            static_cast<int>(oldStatus));
    }

    void TorControl::setTorStatus(TorStatus status)
    {
        auto oldStatus = m_torStatus;
        if (oldStatus == status) return;

        m_torStatus = status;
        emit this->torStatusChanged(
            static_cast<int>(status),
            static_cast<int>(oldStatus));
    }

    void TorControl::setErrorMessage(const QString& msg)
    {
        m_errorMessage = msg;
        this->setStatus(TorControl::Error);
    }
}