#include "rmeglobal.h"

const char *RmeVersion()
{
    return RMEVERSION;
}

QuaZipUsage RmeQuaZipUsage()
{
#if !defined(RME_USE_QUAZIP)
    return QuaZipNotUsed;
#elif defined(QUAZIP_STATIC)
    return QuaZipBundled;
#else
    return QuaZipDynamicLinked;
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
QVersionNumber RmeVersionNumber()
{
    return QVersionNumber::fromString(QStringLiteral(RMEVERSIONNUMBER));
}
#endif
