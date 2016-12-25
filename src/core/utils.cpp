#include "utils.h"

#include <QString>
#include <QMap>

namespace
{
const QString suffix_hd = QStringLiteral("_title_hd");
const QString suffix_ipad = QStringLiteral("_title_ipad");
}

bool hasMp3(const QDir &dir)
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.mp3");

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
        l << QStringLiteral("*.png");

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s;
        s2.chop(4);
        if (s2.toLower() == dir.dirName().toLower())
            return true;
    }

    return false;
}

bool hasSmallPng(const QDir &dir, QString &suffix)
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s.toLower();
        s2.chop(4);
        if (s2.startsWith(dir.dirName().toLower())) {
            s2 = s2.mid(dir.dirName().length()).toLower();
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

bool hasPapaBigPng(const QDir &dir)
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s;
        s2.chop(4);
        if (s2.endsWith(QStringLiteral("_ipad"))) {
            s2.chop(5);
            if (s2.toLower() == dir.dirName().toLower())
                return true;
        }
    }

    return false;
}

bool hasPapaSmallPng(const QDir &dir)
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s;
        s2.chop(4);
        if (s2.endsWith(QStringLiteral("_title_140_90"))) {
            s2.chop(13);
            if (s2.toLower() == dir.dirName().toLower())
                return true;
        }
    }

    return false;
}

ExistNotes existNotes(const QDir &dir)
{
    static QMap<ExistNote, QString> suffixs;
    if (suffixs.isEmpty()) {
        suffixs[IMD_4K_EZ] = QStringLiteral("_4k_ez.imd");
        suffixs[IMD_4K_NM] = QStringLiteral("_4k_nm.imd");
        suffixs[IMD_4K_HD] = QStringLiteral("_4k_hd.imd");
        suffixs[IMD_5K_EZ] = QStringLiteral("_5k_ez.imd");
        suffixs[IMD_5K_NM] = QStringLiteral("_5k_nm.imd");
        suffixs[IMD_5K_HD] = QStringLiteral("_5k_hd.imd");
        suffixs[IMD_6K_EZ] = QStringLiteral("_6k_ez.imd");
        suffixs[IMD_6K_NM] = QStringLiteral("_6k_nm.imd");
        suffixs[IMD_6K_HD] = QStringLiteral("_6k_hd.imd");
        suffixs[MDE_EZ] = QStringLiteral("_Papa_Easy.mde");
        suffixs[MDE_NM] = QStringLiteral("_Papa_Normal.mde");
        suffixs[MDE_HD] = QStringLiteral("_Papa_Hard.mde");
    }

    ExistNotes result;
    for (ExistNote i = IMD_4K_EZ; i <= MDE_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(dir.dirName()).append(suffixs[i]);
        if (dir.exists(file_name))
            result |= i;
    }
    return result;
}

