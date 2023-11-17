
#include <RmEss/RmeGlobal>

// Main test point of this file is to check if
// 1. header file RmeGlobal can be included in C
// 2. these functions have C linkage so that it can be directly called in C

// NOLINTBEGIN

const char *tstRmeVersion()
{
    return RmeVersion();
}

enum QuaZipUsage tstRmeQuaZipUsage()
{
    return RmeQuaZipUsage();
}

enum XxteaUsage tstRmeXxteaUsage()
{
    return RmeXxteaUsage();
}
