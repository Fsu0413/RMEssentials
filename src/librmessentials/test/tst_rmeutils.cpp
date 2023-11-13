
#include <QTest>
#include <RmEss/RmeUtils>

#include <type_traits>

// NOLINTBEGIN

class tst_RmeUtils : public QObject
{
    Q_OBJECT

    static const QString resourcePrefix;

private slots:
    void RmeUtilsHasMp3QDir_data()
    {
        QTest::addColumn<QString>("arg");
        QTest::addColumn<bool>("result");

        QTest::newRow("exist") << QStringLiteral("mp3") << true;
        QTest::newRow("non-exist") << QStringLiteral("empty") << false;
    }
    void RmeUtilsHasMp3QDir()
    {
        QFETCH(QString, arg);
        QFETCH(bool, result);

        QDir d(resourcePrefix + arg);
        bool r = RmeUtils::hasMp3(d);
        QCOMPARE(r, result);
    }

    void RmeUtilsHasBigPngQDir_data()
    {
        QTest::addColumn<QString>("arg");
        QTest::addColumn<bool>("result");

        QTest::newRow("exist") << QStringLiteral("bigpng") << true;
        QTest::newRow("non-exist") << QStringLiteral("empty") << false;
    }
    void RmeUtilsHasBigPngQDir()
    {
        QFETCH(QString, arg);
        QFETCH(bool, result);

        QDir d(resourcePrefix + arg);
        bool r = RmeUtils::hasBigPng(d);
        QCOMPARE(r, result);
    }

    void RmeUtilsHasSmallPngQDirQStringR_data()
    {
        QTest::addColumn<QString>("arg");
        QTest::addColumn<bool>("result");
        QTest::addColumn<QString>("sufResult");

        QTest::newRow("hd") << QStringLiteral("smallpnghd") << true << QStringLiteral("_title_hd");
        QTest::newRow("ipad") << QStringLiteral("smallpngipad") << true << QStringLiteral("_title_ipad");
        QTest::newRow("non-exist") << QStringLiteral("empty") << false << QString();
    }
    void RmeUtilsHasSmallPngQDirQStringR()
    {
        QFETCH(QString, arg);
        QFETCH(bool, result);
        QFETCH(QString, sufResult);

        QDir d(resourcePrefix + arg);
        QString suf;
        bool r = RmeUtils::hasSmallPng(d, suf);
        QCOMPARE(r, result);
        if (result)
            QCOMPARE(suf, sufResult);
    }

    void RmeUtilsHasNewBigPngQDir_data()
    {
        QTest::addColumn<QString>("arg");
        QTest::addColumn<bool>("result");

        QTest::newRow("exist") << QStringLiteral("newbigpng") << true;
        QTest::newRow("non-exist") << QStringLiteral("empty") << false;
    }
    void RmeUtilsHasNewBigPngQDir()
    {
        QFETCH(QString, arg);
        QFETCH(bool, result);

        QDir d(resourcePrefix + arg);
        bool r = RmeUtils::hasNewBigPng(d);
        QCOMPARE(r, result);
    }

    void RmeUtilsHasNewSmallPngQDir_data()
    {
        QTest::addColumn<QString>("arg");
        QTest::addColumn<bool>("result");

        QTest::newRow("exist") << QStringLiteral("newsmallpng") << true;
        QTest::newRow("non-exist") << QStringLiteral("empty") << false;
    }
    void RmeUtilsHasNewSmallPngQDir()
    {
        QFETCH(QString, arg);
        QFETCH(bool, result);

        QDir d(resourcePrefix + arg);
        bool r = RmeUtils::hasNewSmallPng(d);
        QCOMPARE(r, result);
    }

    void RmeUtilsHasPapaBigPngQDir()
    {
        // coverage for hasPapaBigPng
        (void)RmeUtils::hasPapaBigPng(QDir(resourcePrefix + QStringLiteral("newbigpng")));
    }

    void RmeUtilsHasPapaSmallPngQDir_data()
    {
        QTest::addColumn<QString>("arg");
        QTest::addColumn<bool>("result");

        QTest::newRow("exist") << QStringLiteral("papasmallpng") << true;
        QTest::newRow("non-exist") << QStringLiteral("empty") << false;
    }
    void RmeUtilsHasPapaSmallPngQDir()
    {
        QFETCH(QString, arg);
        QFETCH(bool, result);

        QDir d(resourcePrefix + arg);
        bool r = RmeUtils::hasPapaSmallPng(d);
        QCOMPARE(r, result);
    }

    void RmeUtilsExistNotesQdir()
    {
        QDir d(resourcePrefix + QStringLiteral("existnotes"));

        // Due to integer promotion the first enumation value need explicit constructor call
        RmeUtils::ExistNotes result = RmeUtils::ExistNotes(RmeUtils::IMD_4K_EZ) | RmeUtils::MDE_NM | RmeUtils::IMDJSON_5K_EZ | RmeUtils::IMDJSON_5K_NM | RmeUtils::IMDJSON_5K_HD
            | RmeUtils::IMDJSON_6K_EZ | RmeUtils::RMP_6K_HD;

        RmeUtils::ExistNotes r = RmeUtils::existNotes(d);
        QCOMPARE(r, result);
    }

    void RmeUtilsExistImdJsonVersionQdirExistNote_data()
    {
        QTest::addColumn<std::underlying_type_t<RmeUtils::ExistNote> >("arg");
        QTest::addColumn<QString>("result");

        QTest::newRow("121") << static_cast<std::underlying_type_t<RmeUtils::ExistNote> >(RmeUtils::IMDJSON_5K_EZ) << QStringLiteral("1.2.1");
        QTest::newRow("122") << static_cast<std::underlying_type_t<RmeUtils::ExistNote> >(RmeUtils::IMDJSON_5K_NM) << QStringLiteral("1.2.2");
        QTest::newRow("123") << static_cast<std::underlying_type_t<RmeUtils::ExistNote> >(RmeUtils::IMDJSON_5K_HD) << QStringLiteral("1.2.3");
        QTest::newRow("130") << static_cast<std::underlying_type_t<RmeUtils::ExistNote> >(RmeUtils::IMDJSON_6K_EZ) << QStringLiteral("1.3.0");
        QTest::newRow("unexist") << static_cast<std::underlying_type_t<RmeUtils::ExistNote> >(RmeUtils::IMDJSON_4K_EZ) << QString();
        QTest::newRow("Unknown") << static_cast<std::underlying_type_t<RmeUtils::ExistNote> >(RmeUtils::MDE_NM) << QString();
    }
    void RmeUtilsExistImdJsonVersionQdirExistNote()
    {
        QFETCH(std::underlying_type_t<RmeUtils::ExistNote>, arg);
        QFETCH(QString, result);

        QDir d(resourcePrefix + QStringLiteral("existnotes"));
        RmeUtils::ExistNote note = static_cast<RmeUtils::ExistNote>(arg);

        QString r = RmeUtils::existImdJsonVersion(d, note);
        QCOMPARE(r, result);
    }

    void RmeUtilsCalculateSongTimeIntBool_data()
    {
        // Song from Rhythm Master Remastered have different calculation for this value. Need to calculate both.
        // "remastered" false -> old algorithm
        // "remastered" true  -> new algorithm
        QTest::addColumn<int>("arg");
        QTest::addColumn<bool>("remastered");
        QTest::addColumn<QString>("result");

        QTest::newRow("133") << 133 << false << QStringLiteral("0.092361");
        QTest::newRow("144") << 144 << false << QStringLiteral("0.1");
        QTest::newRow("121") << 121 << true << QStringLiteral("0.0014");
        QTest::newRow("253") << 253 << true << QStringLiteral("0.002928");
    }
    void RmeUtilsCalculateSongTimeIntBool()
    {
        QFETCH(int, arg);
        QFETCH(bool, remastered);
        QFETCH(QString, result);

        QString r = RmeUtils::calculateSongTime(arg, remastered);
        QCOMPARE(r, result);
    }

    void RmeUtilsNoteFileNameSuffixExistNote()
    {
        // complexity 1

        QString r = noteFileNameSuffix(RmeUtils::IMD_6K_HD);
        QString result = QStringLiteral("_6k_hd.imd");

        QCOMPARE(r, result);
    }
};

const QString tst_RmeUtils::resourcePrefix = QStringLiteral(":/tst_rmeutils/");

QTEST_GUILESS_MAIN(tst_RmeUtils)
#include "tst_rmeutils.moc"
