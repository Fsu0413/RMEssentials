
#include <RmEss/RmeUncompresser>

#include <QObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

// NOLINTBEGIN

class tst_RmeUncompresser : public QObject
{
    Q_OBJECT

private slots:
    void RmeUncompresserRun_data()
    {
        QTest::addColumn<QString>("zipFile");
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<bool>("success");
        QTest::addColumn<QString>("checkFile");

        QTest::newRow("fail-nonexist-zip") << QStringLiteral("made_by_Fsu0413.zip") << QStringLiteral("made_by_Fsu0413.file") << false << QString();
        QTest::newRow("fail-nonexist-file") << QStringLiteral("t.zip") << QStringLiteral("made_by_Fsu0413.file") << false << QString();
        QTest::newRow("success") << QStringLiteral("t.zip") << QStringLiteral("uncompress_test.txt") << true << QStringLiteral("uncompress_test.txt");
    }
    void RmeUncompresserRun()
    {
        QFETCH(QString, zipFile);
        QFETCH(QString, fileName);
        QFETCH(bool, success);
        QFETCH(QString, checkFile);

        RmeUncompresser unc;
        QSignalSpy spy(&unc, &RmeUncompresser::signalFileFinished);

        QTemporaryDir d;
        d.setAutoRemove(true);
        QString extractedFileName = d.filePath(fileName);
        unc.addFile(QStringLiteral(":/tst_rmeuncompresser/") + zipFile, fileName, extractedFileName);

        // or unc.start / unc.wait
        unc.run();

        if (success) {
            QCOMPARE(spy.length(), 1);
            if (spy.length() > 0) {
                QString actualExtractedFileName = spy.first().first().toString();
                QCOMPARE(actualExtractedFileName, extractedFileName);
                if (actualExtractedFileName == extractedFileName) {
                    QFile f1(actualExtractedFileName);
                    bool op1 = f1.open(QFile::ReadOnly);
                    QCOMPARE(op1, true);
                    if (op1) {
                        QFile f2(QStringLiteral(":/tst_rmeuncompresser/") + checkFile);
                        f2.open(QFile::ReadOnly);
                        QByteArray arr1 = f1.readAll();
                        f1.close();
                        QByteArray arr2 = f2.readAll();
                        f2.close();
                        QCOMPARE(arr1, arr2);
                    }
                }
            }
        } else {
            QCOMPARE(spy.length(), 0);
        }
    }
};

QTEST_GUILESS_MAIN(tst_RmeUncompresser)
#include "tst_rmeuncompresser.moc"
