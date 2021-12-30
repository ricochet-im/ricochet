#pragma once

namespace shims
{
    class TorControlCommand : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(TorControlCommand)

        Q_PROPERTY(bool successful READ isSuccessful CONSTANT)
    public:
        TorControlCommand() = default;
        void onFinished(bool success);

        bool isSuccessful() const;

    signals:
        void finished();

    private:
        bool m_successful = false;
    };
}