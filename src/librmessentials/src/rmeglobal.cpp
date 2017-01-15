#include "rmeglobal.h"

const char *RmeVersion()
{
    return RMEVERSION;
}

QuazipUsage RmeQuazipUsage()
{
#if !defined(RME_USE_QUAZIP)
    return QuazipNotUsed;
#elif defined(QUAZIP_STATIC)
    return QuazipBundled;
#else
    return QuazipDynamicLinked;
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
QVersionNumber RmeVersionNumber()
{
    return QVersionNumber::fromString(QStringLiteral(RMEVERSIONNUMBER));
}
#endif
