#ifndef RMEUTILS_H__INCLUDED
#define RMEUTILS_H__INCLUDED

#include "rmeglobal.h"

#if 0
class LIBRMESSENTIALS_EXPORT RmeUtils
#endif

class QDir;
class QString;

namespace RmeUtils {
enum ExistNote
{
    IMD_4K_EZ = 0x1,
    IMD_4K_NM = 0x2,
    IMD_4K_HD = 0x4,
    IMD_5K_EZ = 0x8,
    IMD_5K_NM = 0x10,
    IMD_5K_HD = 0x20,
    IMD_6K_EZ = 0x40,
    IMD_6K_NM = 0x80,
    IMD_6K_HD = 0x100,
    MDE_EZ = 0x200,
    MDE_NM = 0x400,
    MDE_HD = 0x800,
};
Q_DECLARE_FLAGS(ExistNotes, ExistNote)

LIBRMESSENTIALS_EXPORT bool hasMp3(const QDir &dir);
LIBRMESSENTIALS_EXPORT bool hasBigPng(const QDir &dir);
LIBRMESSENTIALS_EXPORT bool hasSmallPng(const QDir &dir, QString &suffix);
LIBRMESSENTIALS_EXPORT bool hasPapaBigPng(const QDir &dir);
LIBRMESSENTIALS_EXPORT bool hasPapaSmallPng(const QDir &dir);
LIBRMESSENTIALS_EXPORT ExistNotes existNotes(const QDir &dir);

LIBRMESSENTIALS_EXPORT QString calculateSongTime(int gameTime);
}

#endif
