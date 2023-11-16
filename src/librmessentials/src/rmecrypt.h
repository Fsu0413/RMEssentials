#ifndef RMECRYPT_H__INCLUDED
#define RMECRYPT_H__INCLUDED

#include "rmeglobal.h"

#include <QByteArray>

#if 0
class LIBRMESSENTIALS_EXPORT RmeCrypt
#endif

namespace RmeCrypt {
LIBRMESSENTIALS_EXPORT QByteArray decryptXxteaOnly(const QByteArray &encrypted, const QByteArray &decryptKey);
LIBRMESSENTIALS_EXPORT QByteArray decryptFull(const QByteArray &encrypted, const QByteArray &decryptKey);

LIBRMESSENTIALS_EXPORT QByteArray encryptXxteaOnly(const QByteArray &original, const QByteArray &encryptKey);
LIBRMESSENTIALS_EXPORT QByteArray encryptFull(const QByteArray &original, const QByteArray &encryptKey);
}

#endif
