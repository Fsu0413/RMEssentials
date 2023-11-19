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
    IMDJSON_4K_EZ = 0x1000,
    IMDJSON_4K_NM = 0x2000,
    IMDJSON_4K_HD = 0x4000,
    IMDJSON_5K_EZ = 0x8000,
    IMDJSON_5K_NM = 0x10000,
    IMDJSON_5K_HD = 0x20000,
    IMDJSON_6K_EZ = 0x40000,
    IMDJSON_6K_NM = 0x80000,
    IMDJSON_6K_HD = 0x100000,
    RMP_4K_EZ = 0x200000,
    RMP_4K_NM = 0x400000,
    RMP_4K_HD = 0x800000,
    RMP_5K_EZ = 0x1000000,
    RMP_5K_NM = 0x2000000,
    RMP_5K_HD = 0x4000000,
    RMP_6K_EZ = 0x8000000,
    RMP_6K_NM = 0x10000000,
    RMP_6K_HD = 0x20000000,
};
Q_DECLARE_FLAGS(ExistNotes, ExistNote)

LIBRMESSENTIALS_EXPORT bool hasMp3(const QDir &dir);
LIBRMESSENTIALS_EXPORT bool hasBigPng(const QDir &dir);
LIBRMESSENTIALS_EXPORT bool hasSmallPng(const QDir &dir, QString &suffix);
LIBRMESSENTIALS_EXPORT bool hasNewBigPng(const QDir &dir);
LIBRMESSENTIALS_EXPORT bool hasNewSmallPng(const QDir &dir);
Q_ALWAYS_INLINE bool hasPapaBigPng(const QDir &dir)
{
    return hasNewBigPng(dir);
}
LIBRMESSENTIALS_EXPORT bool hasPapaSmallPng(const QDir &dir);
LIBRMESSENTIALS_EXPORT ExistNotes existNotes(const QDir &dir);
LIBRMESSENTIALS_EXPORT QString existImdJsonVersion(const QDir &dir, ExistNote note);

LIBRMESSENTIALS_EXPORT QString calculateSongTime(int gameTime, bool remastered = false);
LIBRMESSENTIALS_EXPORT QString noteFileNameSuffix(ExistNote note);
LIBRMESSENTIALS_EXPORT QByteArray rmpKeyForChart(const QString &fileName);
}

#endif
