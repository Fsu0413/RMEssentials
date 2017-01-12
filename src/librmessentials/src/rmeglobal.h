#ifndef LIBRMEGLOBAL_H
#define LIBRMEGLOBAL_H

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

enum QuazipUsage
{
    QuazipNotUsed = 0x00,
    QuazipUsing = 0x01,
    QuazipBundled = 0x11,
    QuazipDynamicLinked = 0x12
};

#ifdef __cplusplus
extern "C" {
#endif
LIBRMESSENTIALS_EXPORT const char *RmeVersion();
LIBRMESSENTIALS_EXPORT enum QuazipUsage RmeQuazipUsage();
#ifdef __cplusplus
}
#endif

#endif // LIBRMEGLOBAL_H
