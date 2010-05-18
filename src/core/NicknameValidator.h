#ifndef NICKNAMEVALIDATOR_H
#define NICKNAMEVALIDATOR_H

#include <QValidator>

class NicknameValidator : public QValidator
{
    Q_OBJECT
    Q_DISABLE_COPY(NicknameValidator)

public:
    explicit NicknameValidator(QObject *parent = 0);

    virtual State validate(QString &text, int &pos) const;
};

#endif // NICKNAMEVALIDATOR_H
