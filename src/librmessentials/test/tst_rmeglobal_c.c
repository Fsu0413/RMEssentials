
#include <RmEss/RmeGlobal>

// Main test point of this file is to check if header file RmeGlobal can be included in C language

// NOLINTBEGIN

const char *tstRmeVersion()
{
    return RmeVersion();
}

enum QuaZipUsage tstRmeQuaZipUsage()
{
    return RmeQuaZipUsage();
}
