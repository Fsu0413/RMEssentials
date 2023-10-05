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

QVersionNumber RmeVersionNumber()
{
    return QVersionNumber::fromString(QStringLiteral(RMEVERSIONNUMBER));
}
