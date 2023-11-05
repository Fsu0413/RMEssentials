#include <RmEss/RmeChart>

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QTest>
#include <QVariantMap>

#include <type_traits>

// NOLINTBEGIN

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
        QTest::addColumn<unsigned int>("timestamp");
        QTest::addColumn<bool>("isEnd");
        QTest::addColumn<unsigned char>("toTrack");
        QTest::addColumn<unsigned int>("timeDur");
        QTest::addColumn<unsigned char>("attr");
        QTest::addColumn<QByteArray>("result");

        // from canonrock
        const unsigned char SingleKeyResult[11] = {
            0x00U, 0x00U, 0x10U, 0x0EU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("singleKey") << (unsigned char)3 << (unsigned int)3600 << false << (unsigned char)0 << (unsigned int)0 << (unsigned char)0
                                   << QByteArray(reinterpret_cast<const char *>(SingleKeyResult), 11);

        // from jiangnanstyle, 5k
        const unsigned char SingleSlideResult[11] = {
            0x01U, 0x00U, 0x4DU, 0x22U, 0x01U, 0x00U, 0x02U, 0x02U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("singleSlide") << (unsigned char)5 << (unsigned int)74317 << true << (unsigned char)7 << (unsigned int)0 << (unsigned char)3
                                     << QByteArray(reinterpret_cast<const char *>(SingleSlideResult), 11);

        // from buzaiyouyu
        const unsigned char SingleLongPressResult[11] = {
            0x02U, 0x00U, 0x03U, 0x15U, 0x00U, 0x00U, 0x00U, 0x50U, 0x01U, 0x00U, 0x00U,
        };
        QTest::newRow("singleLongPress") << (unsigned char)3 << (unsigned int)5379 << true << (unsigned char)3 << (unsigned int)336 << (unsigned char)3
                                         << QByteArray(reinterpret_cast<const char *>(SingleLongPressResult), 11);

        // from takemyhand
        const unsigned char LongPressMiddleChangeTrackResult[11] = {
            0x21U, 0x00U, 0x7BU, 0x92U, 0x00U, 0x00U, 0x02U, 0x01U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressMiddleChangeTrack") << (unsigned char)5 << (unsigned int)37499 << false << (unsigned char)6 << (unsigned int)0 << (unsigned char)4
                                                    << QByteArray(reinterpret_cast<const char *>(LongPressMiddleChangeTrackResult), 11);

        // from friend63
        const unsigned char LongPressMiddleChangedTrackResult[11] = {
            0x22U, 0x00U, 0xD7U, 0x62U, 0x02U, 0x00U, 0x03U, 0xBBU, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressMiddleChangedTrack") << (unsigned char)6 << (unsigned int)156375 << false << (unsigned char)6 << (unsigned int)187 << (unsigned char)4
                                                     << QByteArray(reinterpret_cast<const char *>(LongPressMiddleChangedTrackResult), 11);

        // from wuqingdeyu, 6k
        const unsigned char LongPressStartChangeTrackResult[11] = {
            0x61U, 0x00U, 0xA2U, 0xA9U, 0x01U, 0x00U, 0x04U, 0x01U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressStartChangeTrack") << (unsigned char)7 << (unsigned int)108962 << false << (unsigned char)8 << (unsigned int)0 << (unsigned char)3
                                                   << QByteArray(reinterpret_cast<const char *>(LongPressStartChangeTrackResult), 11);

        // from friend01, NM
        const unsigned char LongPressStartNoChangeTrackResult[11] = {
            0x62U, 0x00U, 0xA0U, 0x8CU, 0x00U, 0x00U, 0x00U, 0x67U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressStartNoChangeTrack") << (unsigned char)3 << (unsigned int)36000 << false << (unsigned char)3 << (unsigned int)103 << (unsigned char)3
                                                     << QByteArray(reinterpret_cast<const char *>(LongPressStartNoChangeTrackResult), 11);

        // from pingfanzhilu, 6k
        const unsigned char LongPressEndChangeTrackResult[11] = {
            0xA1U, 0x00U, 0x2EU, 0x04U, 0x01U, 0x00U, 0x05U, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
        };
        QTest::newRow("longPressEndChangeTrack") << (unsigned char)8 << (unsigned int)66606 << true << (unsigned char)7 << (unsigned int)0 << (unsigned char)4
                                                 << QByteArray(reinterpret_cast<const char *>(LongPressEndChangeTrackResult), 11);

        // from burn
        const unsigned char LongPressEndNoChangeTrackResult[11] = {
            0xA2U, 0x00U, 0x29U, 0x10U, 0x00U, 0x00U, 0x01U, 0x02U, 0x01U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressEndNoChangeTrackResult") << (unsigned char)4 << (unsigned int)4137 << true << (unsigned char)4 << (unsigned int)258 << (unsigned char)4
                                                         << QByteArray(reinterpret_cast<const char *>(LongPressEndNoChangeTrackResult), 11);
    }
    void RmeChartNoteToImdNoteDoubleC()
    {
        QFETCH(unsigned char, track);
        QFETCH(unsigned int, timestamp);
        QFETCH(bool, isEnd);
        QFETCH(unsigned char, toTrack);
        QFETCH(unsigned int, timeDur);
        QFETCH(unsigned char, attr);
        QFETCH(QByteArray, result);

        RmeChartNote n;
        n.track = track;
        n.timestamp = timestamp;
        n.isEnd = isEnd;
        n.toTrack = toTrack;
        n.timeDur = timeDur;
        n.attr = attr;
        QByteArray arr = n.toImdNote();
        QCOMPARE(arr, result);
    }

    void RmeChartNoteToJsonNoteRmeChartVersionDoubleIntC_data()
    {
        QTest::addColumn<unsigned char>("track");
        QTest::addColumn<unsigned int>("timestamp");
        QTest::addColumn<int>("key");
        QTest::addColumn<bool>("isEnd");
        QTest::addColumn<unsigned char>("toTrack");
        QTest::addColumn<unsigned int>("timeDur");
        QTest::addColumn<unsigned char>("attr");
        QTest::addColumn<std::underlying_type_t<RmeChartVersion::v> >("version");
        QTest::addColumn<double>("bpm");
        QTest::addColumn<int>("idx");
        QTest::addColumn<QVariantMap>("result");

        // from magnetism
        QTest::newRow("121/122") << (unsigned char)3 << (unsigned int)4499 << (int)0 << false << (unsigned char)0 << (unsigned int)0 << (unsigned char)0
                                 << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_2_2) << (double)160.009995 << (int)0
                                 << QVariantMap {
                                        // clang-format off
                                        std::make_pair(QStringLiteral("tick"), 576),
                                        std::make_pair(QStringLiteral("key"), 0),
                                        std::make_pair(QStringLiteral("dur"), 0),
                                        std::make_pair(QStringLiteral("isEnd"), 0),
                                        std::make_pair(QStringLiteral("toTrack"), 0),
                                        std::make_pair(QStringLiteral("volume"), 127),
                                        std::make_pair(QStringLiteral("pan"), 64),
                                        std::make_pair(QStringLiteral("attr"), 0),
                                        // clang-format on
                                    };

        // from yueyawan
        QTest::newRow("123/dur!=0") << (unsigned char)5 << (unsigned int)133102 << (int)0 << false << (unsigned char)5 << (unsigned int)345 << (unsigned char)4
                                    << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_2_3) << (double)87 << (int)420
                                    << QVariantMap {
                                           // clang-format off
                                           std::make_pair(QStringLiteral("tick"), 9263),
                                           std::make_pair(QStringLiteral("key"), 0),
                                           std::make_pair(QStringLiteral("dur"), 24),
                                           std::make_pair(QStringLiteral("isEnd"), 0),
                                           std::make_pair(QStringLiteral("toTrack"), 5),
                                           std::make_pair(QStringLiteral("volume"), 0),
                                           std::make_pair(QStringLiteral("pan"), 0),
                                           std::make_pair(QStringLiteral("attr"), 4),
                                           std::make_pair(QStringLiteral("time"), 133102),
                                           std::make_pair(QStringLiteral("time_dur"), 345),
                                           std::make_pair(QStringLiteral("idx"), 420),
                                           // clang-format on
                                       };

        // from yingxiongsha
        QTest::newRow("130/dur==0") << (unsigned char)4 << (unsigned int)78181 << (int)0 << false << (unsigned char)3 << (unsigned int)1 << (unsigned char)3
                                    << static_cast<std::underlying_type_t<RmeChartVersion::v> >(RmeChartVersion::v1_3_0) << (double)132 << (int)346
                                    << QVariantMap {
                                           // clang-format off
                                           std::make_pair(QStringLiteral("tick"), 8256),
                                           std::make_pair(QStringLiteral("key"), 0),
                                           std::make_pair(QStringLiteral("dur"), 0),
                                           std::make_pair(QStringLiteral("isEnd"), 0),
                                           std::make_pair(QStringLiteral("toTrack"), 3),
                                           std::make_pair(QStringLiteral("volume"), 0),
                                           std::make_pair(QStringLiteral("pan"), 0),
                                           std::make_pair(QStringLiteral("attr"), 3),
                                           std::make_pair(QStringLiteral("time"), 78181),
                                           std::make_pair(QStringLiteral("time_dur"), 1),
                                           std::make_pair(QStringLiteral("idx"), 346),
                                           // clang-format on
                                       };
    }
    void RmeChartNoteToJsonNoteRmeChartVersionDoubleIntC()
    {
        QFETCH(unsigned char, track);
        QFETCH(unsigned int, timestamp);
        QFETCH(int, key);
        QFETCH(bool, isEnd);
        QFETCH(unsigned char, toTrack);
        QFETCH(unsigned int, timeDur);
        QFETCH(unsigned char, attr);
        QFETCH(std::underlying_type_t<RmeChartVersion::v>, version);
        QFETCH(double, bpm);
        QFETCH(int, idx);
        QFETCH(QVariantMap, result);

        RmeChartNote n;
        n.track = track;
        n.timestamp = timestamp;
        n.key = key;
        n.isEnd = isEnd;
        n.toTrack = toTrack;
        n.timeDur = timeDur;
        n.attr = attr;

        QJsonObject ob = n.toJsonNote(static_cast<RmeChartVersion::v>(version), bpm, idx);
        QJsonObject expected = QJsonObject::fromVariantMap(result);
        QCOMPARE(ob, expected);
    }

    void RmeChartNoteOperatorLtRmeChartNoteC_data()
    {
        QTest::addColumn<unsigned char>("track");
        QTest::addColumn<unsigned int>("timestamp");
        QTest::addColumn<bool>("isEnd");
        QTest::addColumn<unsigned char>("toTrack");
        QTest::addColumn<unsigned int>("timeDur");
        QTest::addColumn<unsigned char>("attr");
        QTest::addColumn<unsigned char>("track2");
        QTest::addColumn<unsigned int>("timestamp2");
        QTest::addColumn<bool>("isEnd2");
        QTest::addColumn<unsigned char>("toTrack2");
        QTest::addColumn<unsigned int>("timeDur2");
        QTest::addColumn<unsigned char>("attr2");
        QTest::addColumn<bool>("result");

        // all data from takemyhand
        // Since algorighm unknown the function needs to be modified. More test cases should be added when function modification is done.
        QTest::newRow("tickLess") << (unsigned char)5 << (unsigned int)5716 << false << (unsigned char)0 << (unsigned int)0 << (unsigned char)0 << (unsigned char)4
                                  << (unsigned int)5813 << false << (unsigned char)0 << (unsigned int)0 << (unsigned char)0 << true;
        QTest::newRow("tickMore") << (unsigned char)5 << (unsigned int)5910 << false << (unsigned char)0 << (unsigned int)0 << (unsigned char)0 << (unsigned char)4
                                  << (unsigned int)5813 << false << (unsigned char)0 << (unsigned int)0 << (unsigned char)0 << false;
        QTest::newRow("tickEqualLess") << (unsigned char)4 << (unsigned int)6201 << false << (unsigned char)3 << (unsigned int)0 << (unsigned char)3 << (unsigned char)3
                                       << (unsigned int)6201 << true << (unsigned char)3 << (unsigned int)775 << (unsigned char)4 << true;
        QTest::newRow("tickEqualMore") << (unsigned char)6 << (unsigned int)6201 << true << (unsigned char)6 << (unsigned int)775 << (unsigned char)4 << (unsigned char)5
                                       << (unsigned int)6201 << false << (unsigned char)6 << (unsigned int)0 << (unsigned char)3 << false;
        QTest::newRow("tickEqualTrack") << (unsigned char)4 << (unsigned int)7557 << true << (unsigned char)3 << (unsigned int)0 << (unsigned char)3 << (unsigned char)5
                                        << (unsigned int)7557 << true << (unsigned char)6 << (unsigned int)0 << (unsigned char)3 << true;
    }
    void RmeChartNoteOperatorLtRmeChartNoteC()
    {
        QFETCH(unsigned char, track);
        QFETCH(unsigned int, timestamp);
        QFETCH(bool, isEnd);
        QFETCH(unsigned char, toTrack);
        QFETCH(unsigned int, timeDur);
        QFETCH(unsigned char, attr);
        QFETCH(unsigned char, track2);
        QFETCH(unsigned int, timestamp2);
        QFETCH(bool, isEnd2);
        QFETCH(unsigned char, toTrack2);
        QFETCH(unsigned int, timeDur2);
        QFETCH(unsigned char, attr2);
        QFETCH(bool, result);

        RmeChartNote n;
        n.track = track;
        n.timestamp = timestamp;
        n.isEnd = isEnd;
        n.toTrack = toTrack;
        n.timeDur = timeDur;
        n.attr = attr;
        RmeChartNote n2;
        n2.track = track2;
        n2.timestamp = timestamp2;
        n2.isEnd = isEnd2;
        n2.toTrack = toTrack2;
        n2.timeDur = timeDur2;
        n2.attr = attr2;

        QCOMPARE(n < n2, result);
    }

    void RmeChartNoteOperatorEqRmeChartNoteC()
    {
        // complexity 1
        // data from takemyhand

        RmeChartNote n;
        n.track = 5;
        n.timestamp = 141278;
        n.isEnd = true;
        n.toTrack = 6;
        n.timeDur = 0;
        n.attr = 3;
        RmeChartNote n2;
        n2.track = 5;
        n2.timestamp = 141278;
        n2.isEnd = true;
        n2.toTrack = 6;
        n2.timeDur = 0;
        n2.attr = 3;

        QVERIFY(n == n2);
    }

    void RmeChartNoteFromImdNoteQByteArrayDoubleBoolPS_data()
    {
        QTest::addColumn<QByteArray>("note");
        QTest::addColumn<unsigned char>("track");
        QTest::addColumn<unsigned int>("timestamp");
        QTest::addColumn<bool>("isEnd");
        QTest::addColumn<unsigned char>("toTrack");
        QTest::addColumn<unsigned int>("timeDur");
        QTest::addColumn<unsigned char>("attr");
        QTest::addColumn<bool>("ok");

        QTest::newRow("invalid/length") << QByteArray() << (unsigned char)1 << (unsigned int)1 << false << (unsigned char)1 << (unsigned int)1 << (unsigned char)1 << false;

        const unsigned char InvalidType[11] = {
            0x11U, 0x00U, 0x62U, 0x12U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("invalid/type") << QByteArray(reinterpret_cast<const char *>(InvalidType), 11) << (unsigned char)6 << (unsigned int)4706 << false << (unsigned char)0
                                      << (unsigned int)0 << (unsigned char)0 << false;

        // following data are from buzaiyouyu

        const unsigned char SingleKey[11] = {
            0x00U, 0x00U, 0x62U, 0x12U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("singleKey") << QByteArray(reinterpret_cast<const char *>(SingleKey), 11) << (unsigned char)6 << (unsigned int)4706 << false << (unsigned char)0
                                   << (unsigned int)0 << (unsigned char)0 << true;

        const unsigned char SingleSlide[11] = {
            0x01U, 0x00U, 0xE4U, 0x1CU, 0x00U, 0x00U, 0x02U, 0x01U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("singleSlide") << QByteArray(reinterpret_cast<const char *>(SingleSlide), 11) << (unsigned char)5 << (unsigned int)7396 << true << (unsigned char)6
                                     << (unsigned int)0 << (unsigned char)3 << true;

        const unsigned char SingleLongPress[11] = {
            0x02U, 0x00U, 0x03U, 0x15U, 0x00U, 0x00U, 0x00U, 0x50U, 0x01U, 0x00U, 0x00U,
        };
        QTest::newRow("singleLongPress") << QByteArray(reinterpret_cast<const char *>(SingleLongPress), 11) << (unsigned char)3 << (unsigned int)5379 << true << (unsigned char)3
                                         << (unsigned int)336 << (unsigned char)3 << true;

        const unsigned char LongPressMiddleChangeTrack[11] = {
            0x21U, 0x00U, 0xB1U, 0x0CU, 0x00U, 0x00U, 0x03U, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
        };
        QTest::newRow("longPressMiddleChangeTrack") << QByteArray(reinterpret_cast<const char *>(LongPressMiddleChangeTrack), 11) << (unsigned char)6 << (unsigned int)3249 << false
                                                    << (unsigned char)5 << (unsigned int)0 << (unsigned char)4 << true;

        const unsigned char LongPressMiddleChangedTrack[11] = {
            0x22U, 0x00U, 0x21U, 0x0DU, 0x00U, 0x00U, 0x01U, 0x70U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressMiddleChangedTrack") << QByteArray(reinterpret_cast<const char *>(LongPressMiddleChangedTrack), 11) << (unsigned char)4 << (unsigned int)3361
                                                     << false << (unsigned char)4 << (unsigned int)112 << (unsigned char)4 << true;

        const unsigned char LongPressStartChangeTrack[11] = {
            0x61U, 0x00U, 0xA6U, 0x2CU, 0x00U, 0x00U, 0x01U, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
        };
        QTest::newRow("longPressStartChangeTrack") << QByteArray(reinterpret_cast<const char *>(LongPressStartChangeTrack), 11) << (unsigned char)4 << (unsigned int)11430 << false
                                                   << (unsigned char)3 << (unsigned int)0 << (unsigned char)3 << true;

        const unsigned char LongPressStartNoChangeTrack[11] = {
            0x62U, 0x00U, 0x41U, 0x0CU, 0x00U, 0x00U, 0x03U, 0x70U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressStartNoChangeTrack") << QByteArray(reinterpret_cast<const char *>(LongPressStartNoChangeTrack), 11) << (unsigned char)6 << (unsigned int)3137
                                                     << false << (unsigned char)6 << (unsigned int)112 << (unsigned char)3 << true;

        const unsigned char LongPressEndChangeTrack[11] = {
            0xA1U, 0x00U, 0x73U, 0x82U, 0x00U, 0x00U, 0x02U, 0x01U, 0x00U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressEndChangeTrack") << QByteArray(reinterpret_cast<const char *>(LongPressEndChangeTrack), 11) << (unsigned char)5 << (unsigned int)33395 << true
                                                 << (unsigned char)6 << (unsigned int)0 << (unsigned char)4 << true;

        const unsigned char LongPressEndNoChangeTrack[11] = {
            0xA2U, 0x00U, 0x35U, 0x92U, 0x00U, 0x00U, 0x03U, 0x11U, 0x03U, 0x00U, 0x00U,
        };
        QTest::newRow("longPressEndNoChangeTrack") << QByteArray(reinterpret_cast<const char *>(LongPressEndNoChangeTrack), 11) << (unsigned char)6 << (unsigned int)37429 << true
                                                   << (unsigned char)6 << (unsigned int)785 << (unsigned char)4 << true;
    }
    void RmeChartNoteFromImdNoteQByteArrayDoubleBoolPS()
    {
        QFETCH(QByteArray, note);
        QFETCH(unsigned char, track);
        QFETCH(unsigned int, timestamp);
        QFETCH(bool, isEnd);
        QFETCH(unsigned char, toTrack);
        QFETCH(unsigned int, timeDur);
        QFETCH(unsigned char, attr);
        QFETCH(bool, ok);

        bool isOk = false;
        RmeChartNote n = RmeChartNote::fromImdNote(note, &isOk);
        QCOMPARE(isOk, ok);
        if (ok) {
            QCOMPARE(n.track, track);
            QCOMPARE(n.timestamp, timestamp);
            QCOMPARE(n.isEnd, isEnd);
            QCOMPARE(n.toTrack, toTrack);
            QCOMPARE(n.timeDur, timeDur);
            QCOMPARE(n.attr, attr);
        }
    }

    void RmeChartNoteFromImdNoteQByteArrayDoubleBoolPNS()
    {
        // coverage for ok == nullptr
        // since it can't be data driven so call it separately

        RmeChartNote::fromImdNote({});
    }

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
