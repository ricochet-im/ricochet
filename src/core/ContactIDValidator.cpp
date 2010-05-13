#include "ContactIDValidator.h"
#include <QRegExp>

ContactIDValidator::ContactIDValidator(QObject *parent)
    : QValidator(parent)
{
}

QValidator::State ContactIDValidator::validate(QString &text, int &pos) const
{
    Q_UNUSED(pos);

    /* [a-z2-7]{16}@TorIM */
    for (int i = 0; i < qMin(text.size(), 16); ++i)
    {
        char c = text[i].toLatin1();
        if (!((c >= 'a' && c <= 'z') || (c >= '2' && c <= '7')))
            return QValidator::Invalid;
    }

    if (text.size() < 16)
        return QValidator::Intermediate;
    else if (text.size() > 22)
        return QValidator::Invalid;

    QString suffix = QLatin1String("@TorIM");
    suffix.truncate(text.size() - 16);

    if (QString::compare(text.mid(16), suffix, Qt::CaseInsensitive) != 0)
        return QValidator::Invalid;

    if (suffix.size() != 6)
        return QValidator::Intermediate;

    return QValidator::Acceptable;
}

bool ContactIDValidator::isValidID(const QString &text)
{
    QRegExp regex(QLatin1String("^[a-z2-7]{16}@TorIM$"), Qt::CaseInsensitive);
    return regex.exactMatch(text);
}

QString ContactIDValidator::hostnameFromID(const QString &ID)
{
    if (!isValidID(ID))
        return QString();

    return ID.mid(0, 16) + QLatin1String(".onion");
}
