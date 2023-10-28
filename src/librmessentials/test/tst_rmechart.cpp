#include <RmEss/RmeChart>

#include <QObject>
#include <QTest>

#include <type_traits>

class tst_RmeChart : public QObject
{
    Q_OBJECT

private slots:

    void RmeChartVersionCtorV()
    {
        RmeChartVersion v(RmeChartVersion::v1_2_1);
        QCOMPARE(v, RmeChartVersion::v1_2_1);
    }

    void RmeChartVersionOperatorAsV()
    {
        RmeChartVersion v(RmeChartVersion::v1_2_1);
        v = RmeChartVersion::v1_2_2;
        QCOMPARE(v, RmeChartVersion::v1_2_2);
    }

    void RmeChartVersionOperatorV()
    {
        RmeChartVersion v(RmeChartVersion::v1_2_1);
        QCOMPARE(v, RmeChartVersion::v1_2_1);
    }

    void RmeChartVersionCtorQString_data()
    {
        QTest::addColumn<QString>("arg");
        QTest::addColumn<std::underlying_type_t<RmeChartVersion::v> >("result");

        QTest::newRow("121") << QStringLiteral("1.2.1") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_2_1);
        QTest::newRow("122") << QStringLiteral("1.2.2") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_2_2);
        QTest::newRow("123") << QStringLiteral("1.2.3") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_2_3);
        QTest::newRow("130") << QStringLiteral("1.3.0") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_3_0);
        QTest::newRow("Unknown") << QStringLiteral("Made by Fsu0413") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::vUnknown);
    }
    void RmeChartVersionCtorQString()
    {
        QFETCH(QString, arg);
        QFETCH(std::underlying_type_t<RmeChartVersion::v>, result);
        RmeChartVersion v(arg);
        QCOMPARE(v, static_cast<RmeChartVersion::v>(result));
    }

    void RmeChartVersionToStringC_data()
    {
        QTest::addColumn<std::underlying_type_t<RmeChartVersion::v> >("arg");
        QTest::addColumn<QString>("result");

        QTest::newRow("121") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_2_1) << QStringLiteral("1.2.1");
        QTest::newRow("122") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_2_2) << QStringLiteral("1.2.2");
        QTest::newRow("123") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_2_3) << QStringLiteral("1.2.3");
        QTest::newRow("130") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_3_0) << QStringLiteral("1.3.0");
        QTest::newRow("Imd") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::vImd) << QStringLiteral("1.2.2");
        QTest::newRow("Unknown") << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::vUnknown) << QStringLiteral("1.2.1");
    }
    void RmeChartVersionToStringC()
    {
        QFETCH(std::underlying_type_t<RmeChartVersion::v>, arg);
        QFETCH(QString, result);
        RmeChartVersion v(static_cast<RmeChartVersion::v>(arg));
        QCOMPARE(v.toString(), result);
    }

    //    void RmeChartNoteToImdNoteDoubleC()
    //    {
    //    }
    //    void RmeChartNoteToJsonNoteRmeChartVersionDoubleIntC()
    //    {
    //    }
    //    void RmeChartNoteOperatorLtRmeChartNoteC()
    //    {
    //    }
    //    void RmeChartNoteOperatorEqRmeChartNoteC()
    //    {
    //    }
    //    void RmeChartNoteFromImdNoteQByteArrayDoubleBoolPS()
    //    {
    //    }
    //    void RmeChartNoteFromJsonNoteQJsonObjectUnsignedCharBoolPS()
    //    {
    //    }

    //    void RmeChartToImdC()
    //    {
    //    }
    //    void RmeChartToJsonRmeChartVersionC()
    //    {
    //    }
    //    void RmeChartFromImdQByteArrayBoolPS()
    //    {
    //    }
    //    void RmeChartFromJsonQJsonObjectBoolPS()
    //    {
    //    }
};

QTEST_GUILESS_MAIN(tst_RmeChart)
#include "tst_rmechart.moc"
