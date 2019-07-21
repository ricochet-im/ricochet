#include <QtTest>

class TestTemplateCase : public QObject
{
    Q_OBJECT

private slots:
    void test_a();
};

void TestTemplateCase::test_a()
{
    QVERIFY(false);
}

QTEST_MAIN(TestTemplateCase)
#include "tst_template.moc"
