#ifndef RMEGLOBAL_H__INCLUDED
#define RMEGLOBAL_H__INCLUDED

#include <QVersionNumber>
#include <QtGlobal>

#ifdef __cplusplus
#include <QDebug>
#endif
// We assume the byte order is little endian.
// Force a compile error when compiling for a machine of big endian.
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#error "Big endian is not supported"
#endif

#if 0
class LIBRMESSENTIALS_EXPORT RmeGlobal
#endif

#ifdef LIBRMESSENTIALS_STATIC
#define LIBRMESSENTIALS_EXPORT
#else
#ifndef LIBRMESSENTIALS_BUILD
#define LIBRMESSENTIALS_EXPORT Q_DECL_IMPORT
#else
#define LIBRMESSENTIALS_EXPORT Q_DECL_EXPORT
#endif
#endif

enum QuaZipUsage
{
    QuaZipNotUsed = 0x00,
    QuaZipUsing = 0x01,
    QuaZipBundled = 0x11,
    QuaZipDynamicLinked = 0x12
};

#ifdef __cplusplus
extern "C" {
#endif
LIBRMESSENTIALS_EXPORT const char *RmeVersion();
LIBRMESSENTIALS_EXPORT enum QuaZipUsage RmeQuaZipUsage();
#ifdef __cplusplus
}
LIBRMESSENTIALS_EXPORT QVersionNumber RmeVersionNumber();
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#error "RMEssentials only support Qt >= 5.15"
#elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0) && QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
#error "RMEssentials for Qt 6 only support Qt >= 6.2 since important functionality is missing in Qt 6.0 and 6.1"
#endif

#endif
