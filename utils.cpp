#include "utils.h"
#include <QMap>

#include <QDir>

namespace {
const char *suffix_hd = "_title_hd";
const char *suffix_ipad = "_title_ipad";
}

bool hasMp3(const QDir &dir)
{
    static QStringList l;
    if (l.isEmpty())
        l << "*.mp3";

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s;
        s2.chop(4);
        if (s2.toLower() == dir.dirName().toLower())
            return true;
    }

    return false;
}

bool hasBigPng(const QDir &dir)
{
    static QStringList l;
    if (l.isEmpty())
        l << "*.png";

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s;
        s2.chop(4);
        if (s2.toLower() == dir.dirName().toLower())
            return true;
    }

    return false;
}

bool hasSmallPng(const QDir &dir, const char * &suffix)
{
    static QStringList l;
    if (l.isEmpty())
        l << "*.png";

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s;
        s2.chop(4);
        if (s2.startsWith(dir.dirName().toLower())) {
            s2 = s2.mid(dir.dirName().length());
            if (s2 == suffix_hd) {
                suffix = suffix_hd;
                return true;
            } else if (s2 == suffix_ipad) {
                suffix = suffix_ipad;
                return true;
            }
        }
    }

    return false;
}

ExistImds existImds(const QDir &dir)
{
    static QMap<ExistImd, QString> suffixs;
    if (suffixs.isEmpty()) {
        suffixs[IMD_4K_EZ] = "_4k_ez";
        suffixs[IMD_4K_NM] = "_4k_nm";
        suffixs[IMD_4K_HD] = "_4k_hd";
        suffixs[IMD_5K_EZ] = "_5k_ez";
        suffixs[IMD_5K_NM] = "_5k_nm";
        suffixs[IMD_5K_HD] = "_5k_hd";
        suffixs[IMD_6K_EZ] = "_6k_ez";
        suffixs[IMD_6K_NM] = "_6k_nm";
        suffixs[IMD_6K_HD] = "_6k_hd";
    }

    /*
    static QStringList l;
    if (l.isEmpty())
        l << "*.imd";
    */

    ExistImds result;
    for (ExistImd i = IMD_4K_EZ; i <= IMD_6K_HD; i = static_cast<ExistImd>(i << 1)) {
        QString file_name;
        file_name.append(dir.dirName()).append(suffixs[i]).append(".imd");
        if (dir.exists(file_name))
            result |= i;
    }

    /*
    foreach (const QString &s, suffixs) {
        QString file_name;
        file_name.append(dir.dirName()).append(s).append(".imd");
        if (dir.exists(file_name))
            result |= suffixs.key(s);
    }
    */
    return result;
}

