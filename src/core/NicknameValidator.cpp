#include "NicknameValidator.h"

NicknameValidator::NicknameValidator(QObject *parent)
    : QValidator(parent)
{
}

QValidator::State NicknameValidator::validate(QString &text, int &pos) const
{
    Q_UNUSED(pos);

    if (text.size() < 1)
        return Intermediate;
    else if (text.size() > 15)
        return Invalid;

    bool nonws = false, wssuf = false;
    for (QString::iterator it = text.begin(); it != text.end(); ++it)
    {
        if (!it->isPrint())
            return Invalid;

        if (it->isSpace())
        {
            if (!nonws)
                return Invalid;
            wssuf = true;
        }
        else
        {
            nonws = true;
            wssuf = false;
        }
    }

    if (!nonws)
        return Invalid;
    else if (wssuf)
        return Intermediate;

    return Acceptable;
}
