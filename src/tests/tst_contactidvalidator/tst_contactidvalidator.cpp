#include <QtTest>
#include <QString>
#include <QValidator>

#include "core/ContactIDValidator.h"

class TestContactIDValidator : public QObject
{
    Q_OBJECT

private slots:
    void test_validate();
};

ContactIDValidator validator((QObject*)NULL);

void TestContactIDValidator::test_validate()
{
    QString text = "hi";
    int pos = 0;
    QCOMPARE(validator.validate(text, pos), QValidator::Invalid);

    text = "ricochet:iou53ffunpweuzy5";
    pos = 0;
    QCOMPARE(validator.validate(text, pos), QValidator::Acceptable);
}

QTEST_APPLESS_MAIN(TestContactIDValidator)
#include "tst_contactidvalidator.moc"
