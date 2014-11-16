#ifndef __UTILS_H__
#define __UTILS_H__

#include <QFlag>

class QDir;

enum ExistImd {
    IMD_4K_EZ = 0x1,
    IMD_4K_NM = 0x2,
    IMD_4K_HD = 0x4,
    IMD_5K_EZ = 0x8,
    IMD_5K_NM = 0x10,
    IMD_5K_HD = 0x20,
    IMD_6K_EZ = 0x40,
    IMD_6K_NM = 0x80,
    IMD_6K_HD = 0x100
};
Q_DECLARE_FLAGS(ExistImds, ExistImd)

bool hasMp3(const QDir &dir);
bool hasBigPng(const QDir &dir);
bool hasSmallPng(const QDir &dir, const char * &suffix);
ExistImds existImds(const QDir &dir);


#endif // __UTILS_H__
