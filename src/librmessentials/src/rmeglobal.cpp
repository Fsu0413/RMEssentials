#include "rmeglobal.h"

const char *RmeVersion()
{
    return RMEVERSION;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
QVersionNumber RmeVersionNumber()
{
    return QVersionNumber::fromString(QStringLiteral(RMEVERSIONNUMBER));
}
#endif
