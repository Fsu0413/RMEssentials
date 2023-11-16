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

XxteaUsage RmeXxteaUsage()
{
#ifdef RMESSENTIALS_BUILD_STATIC_XXTEA
    return XxteaBundled;
#else
    return XxteaDynamicLinked;
#endif
}

QVersionNumber RmeVersionNumber()
{
    return QVersionNumber::fromString(QStringLiteral(RMEVERSIONNUMBER));
}
