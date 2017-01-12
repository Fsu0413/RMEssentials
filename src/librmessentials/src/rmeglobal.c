#include "rmeglobal.h"

const char *RmeVersion()
{
    return RMEVERSION;
}

enum QuazipUsage RmeQuazipUsage()
{
#if !defined(RME_USE_QUAZIP)
    return QuazipNotUsed;
#elif defined(QUAZIP_STATIC)
    return QuazipBundled;
#else
    return QuazipDynamicLinked;
#endif
}
