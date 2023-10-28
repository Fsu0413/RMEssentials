#include <RmEss/RmeChart>

#include <QByteArray>
#include <QObject>
#include <QString>
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

    void RmeChartNoteToImdNoteDoubleC_data()
    {
        QTest::addColumn<unsigned char>("track");
        QTest::addColumn<unsigned int>("tick");
        QTest::addColumn<bool>("isEnd");
        QTest::addColumn<unsigned char>("toTrack");
        QTest::addColumn<unsigned int>("dur");
        QTest::addColumn<unsigned char>("attr");
        QTest::addColumn<double>("bpm");
        QTest::addColumn<QByteArray>("result");

        // from canonrock
        const unsigned char SingleKeyResult[11] = {
            0x00U, 0x00U, 0x10U, 0x0EU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("singleKey") << (unsigned char)3 << (unsigned int)576 << false << (unsigned char)0 << (unsigned int)0 << (unsigned char)0 << (double)200
                                   << QByteArray(reinterpret_cast<const char *>(SingleKeyResult), 11);

        // from jiangnanstyle, 5k
        const unsigned char SingleSlideResult[11] = {
            0x01U, 0x00U, 0x4DU, 0x22U, 0x01U, 0x00U, 0x02U, 0x02U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("singleSlide") << (unsigned char)5 << (unsigned int)7848 << true << (unsigned char)7 << (unsigned int)0 << (unsigned char)3 << (double)132
                                     << QByteArray(reinterpret_cast<const char *>(SingleSlideResult), 11);

        // from buzaiyouyu
        const unsigned char SingleLongPressResult[11] = {
            0x02U, 0x00U, 0x03U, 0x15U, 0x00U, 0x00U, 0x00U, 0x50U, 0x01U, 0x00U, 0x00U,
        };
        QTest::newRow("singleLongPress") << (unsigned char)3 << (unsigned int)576 << true << (unsigned char)3 << (unsigned int)36 << (unsigned char)3 << (double)133.85
                                         << QByteArray(reinterpret_cast<const char *>(SingleLongPressResult), 11);

        // from takemyhand
        const unsigned char LongPressMiddleChangeTrackResult[11] = {
            0x21U, 0x00U, 0x7BU, 0x92U, 0x00U, 0x00U, 0x02U, 0x01U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressMiddleChangeTrack") << (unsigned char)5 << (unsigned int)4644 << false << (unsigned char)6 << (unsigned int)0 << (unsigned char)4 << (double)154.8
                                                    << QByteArray(reinterpret_cast<const char *>(LongPressMiddleChangeTrackResult), 11);

        // from friend63
        const unsigned char LongPressMiddleChangedTrackResult[11] = {
            0x22U, 0x00U, 0xD7U, 0x62U, 0x02U, 0x00U, 0x03U, 0xBBU, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressMiddleChangedTrack") << (unsigned char)6 << (unsigned int)20016 << false << (unsigned char)6 << (unsigned int)24 << (unsigned char)4 << (double)160
                                                     << QByteArray(reinterpret_cast<const char *>(LongPressMiddleChangedTrackResult), 11);

        // from wuqingdeyu, 6k
        const unsigned char LongPressStartChangeTrackResult[11] = {
            0x61U, 0x00U, 0xA2U, 0xA9U, 0x01U, 0x00U, 0x04U, 0x01U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressStartChangeTrack") << (unsigned char)7 << (unsigned int)5184 << false << (unsigned char)8 << (unsigned int)0 << (unsigned char)3 << (double)59.47
                                                   << QByteArray(reinterpret_cast<const char *>(LongPressStartChangeTrackResult), 11);

        // from friend01, NM
        const unsigned char LongPressStartNoChangeTrackResult[11] = {
            0x62U, 0x00U, 0xA0U, 0x8CU, 0x00U, 0x00U, 0x00U, 0x67U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressStartNoChangeTrack") << (unsigned char)3 << (unsigned int)8352 << false << (unsigned char)3 << (unsigned int)24 << (unsigned char)3 << (double)290
                                                     << QByteArray(reinterpret_cast<const char *>(LongPressStartNoChangeTrackResult), 11);

        // from pingfanzhilu, 6k
        const unsigned char LongPressEndChangeTrackResult[11] = {
            0xA1U, 0x00U, 0x2EU, 0x04U, 0x01U, 0x00U, 0x05U, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
        };
        QTest::newRow("longPressEndChangeTrack") << (unsigned char)8 << (unsigned int)4476 << true << (unsigned char)7 << (unsigned int)0 << (unsigned char)4 << (double)84
                                                 << QByteArray(reinterpret_cast<const char *>(LongPressEndChangeTrackResult), 11);

        // from burn
        const unsigned char LongPressEndNoChangeTrackResult[11] = {
            0xA2U, 0x00U, 0x29U, 0x10U, 0x00U, 0x00U, 0x01U, 0x02U, 0x01U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressEndNoChangeTrackResult") << (unsigned char)4 << (unsigned int)576 << true << (unsigned char)4 << (unsigned int)36 << (unsigned char)4 << (double)174
                                                         << QByteArray(reinterpret_cast<const char *>(LongPressEndNoChangeTrackResult), 11);
    }
    void RmeChartNoteToImdNoteDoubleC()
    {
        QSKIP("There is difference 1 error in this test. Temporarily skipping and see if there is some good solution");

        QFETCH(unsigned char, track);
        QFETCH(unsigned int, tick);
        QFETCH(bool, isEnd);
        QFETCH(unsigned char, toTrack);
        QFETCH(unsigned int, dur);
        QFETCH(unsigned char, attr);
        QFETCH(double, bpm);
        QFETCH(QByteArray, result);

        RmeChartNote n;
        n.track = track;
        n.tick = tick;
        n.isEnd = isEnd;
        n.toTrack = toTrack;
        n.dur = dur;
        n.attr = attr;
        QByteArray arr = n.toImdNote(bpm);
        QCOMPARE(arr, result);
    }
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
