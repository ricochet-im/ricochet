#pragma once

namespace shims
{
    class ContactUser;
    class ContactIDValidator : public QRegularExpressionValidator
    {
        Q_OBJECT
        Q_DISABLE_COPY(ContactIDValidator)
    public:
        ContactIDValidator(QObject *parent = 0);

        virtual void fixup(QString &text) const;
        virtual State validate(QString &text, int &pos) const;

        Q_INVOKABLE shims::ContactUser *matchingContact(const QString &text) const;
        Q_INVOKABLE bool matchesIdentity(const QString &text) const;
    signals:
        void failed() const;
    };
}