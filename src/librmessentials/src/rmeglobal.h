#ifndef LIBRMEGLOBAL_H
#define LIBRMEGLOBAL_H

#include <QtGlobal>

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

#endif // LIBRMEGLOBAL_H
