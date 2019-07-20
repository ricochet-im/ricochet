#include <QtTest>

class TestContactIDValidator : public QObject
{
    Q_OBJECT

private slots:
    void load();
};

void TestContactIDValidator::load()
{
    QVERIFY(false);
}

#include "test_contactIDValidator.moc"
