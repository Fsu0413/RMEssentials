#ifndef RMEGLOBAL_H__INCLUDED
#define RMEGLOBAL_H__INCLUDED

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#error "RMEssentials only support Qt >= 5.15"
#elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0) && QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
#error "RMEssentials for Qt 6 only support Qt >= 6.2 since important functionality is missing in Qt 6.0 and 6.1"
#endif

#ifdef __cplusplus
#include <QDebug>
#include <QVersionNumber>

#include <limits>

// We assume the byte order is little endian.
// Force a compile error when compiling for a machine of big endian.
#if __cplusplus >= 202002L
#include <bit>
static_assert(std::endian::native == std::endian::little);
#else
static_assert(Q_BYTE_ORDER == Q_LITTLE_ENDIAN);
#endif

// It seems all C / C++ compatible program should use an IEEE-754 compatible double implementation
// The RM IMD registers BPM in IEEE-754 formaat so it seems like we can use memcpy and / or reinterpret_cast for converting it with double
// Static-asserting the double type is of size 8 and IEEE-754 compatible here for memcpy-ing or reinterpret_cast-ing
static_assert(sizeof(double) == 8);
static_assert(std::numeric_limits<double>::is_iec559);

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

enum XxteaUsage
{
    XxteaNotUsed = 0x00,
    XxteaUsing = 0x01,
    XxteaBundled = 0x11,
    XxteaDynamicLinked = 0x12,
};

#ifdef __cplusplus
extern "C" {
#endif
LIBRMESSENTIALS_EXPORT const char *RmeVersion();
LIBRMESSENTIALS_EXPORT enum QuaZipUsage RmeQuaZipUsage();
LIBRMESSENTIALS_EXPORT enum XxteaUsage RmeXxteaUsage();
#ifdef __cplusplus
}
LIBRMESSENTIALS_EXPORT QVersionNumber RmeVersionNumber();
#endif

#endif
