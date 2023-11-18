
#include <RmEss/RmeGlobal>

#include <QString>
#include <QTest>

// NOLINTBEGIN

extern "C" {
const char *tstRmeVersion();
QuaZipUsage tstRmeQuaZipUsage();
XxteaUsage tstRmeXxteaUsage();
}

class tst_RmeGlobal : public QObject
{
    Q_OBJECT

private slots:

    // all these functions have complexity 1...
    // following 2 functions are called from C interface and C++ interface

    void RmeGlobalRmeVersion()
    {
        const char *version = tstRmeVersion();
        QCOMPARE(QString::fromLatin1(version), QString::fromLatin1(RmeVersion()));
    }

    void RmeGlobalRmeQuaZipUsage()
    {
        QuaZipUsage usage = tstRmeQuaZipUsage();
        QCOMPARE(usage, RmeQuaZipUsage());
    }

    void RmeGlobalRmeXxteaUsage()
    {
        XxteaUsage usage = tstRmeXxteaUsage();
        QCOMPARE(usage, RmeXxteaUsage());
    }

    // C++ only interface, called for coverage
    void RmeGlobalRmeVersionNumber()
    {
        QVersionNumber v = RmeVersionNumber();
        Q_UNUSED(v);
    }
};

QTEST_GUILESS_MAIN(tst_RmeGlobal)
#include "tst_rmeglobal.moc"
