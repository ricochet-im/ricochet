#ifndef CONTACTIDVALIDATOR_H
#define CONTACTIDVALIDATOR_H

#include <QValidator>

class ContactIDValidator : public QValidator
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactIDValidator)

public:
    ContactIDValidator(QObject *parent = 0);

    static bool isValidID(const QString &text);
    static QString hostnameFromID(const QString &ID);
    static QString idFromHostname(const QString &hostname);
    static QString idFromHostname(const QByteArray &hostname) { return idFromHostname(QString::fromLatin1(hostname)); }

    virtual State validate(QString &text, int &pos) const;
};

#endif // CONTACTIDVALIDATOR_H
