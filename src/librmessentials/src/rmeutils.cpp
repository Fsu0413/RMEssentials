#include "rmeutils.h"

#include <QDir>
#include <QGlobalStatic>
#include <QMap>
#include <QString>

#include <tuple>

bool RmeUtils::hasMp3(const QDir &dir)
{
    static const QStringList l {QStringLiteral("*.mp3")};

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s;
        s2.chop(4);
        if (s2.toLower() == dir.dirName().toLower())
            return true;
    }

    return false;
}

bool RmeUtils::hasBigPng(const QDir &dir)
{
    static const QStringList l {QStringLiteral("*.png")};

    foreach (const QString &s, dir.entryList(l)) {
        QString s2 = s;
        s2.chop(4);
        if (s2.toLower() == dir.dirName().toLower())
            return true;
    }

    return false;
}

bool RmeUtils::hasSmallPng(const QDir &dir, QString &suffix)
{
    static const QStringList l {QStringLiteral("*.png")};

    static const QString suffix_hd = QStringLiteral("_title_hd");
    static const QString suffix_ipad = QStringLiteral("_title_ipad");

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

bool RmeUtils::hasPapaBigPng(const QDir &dir)
{
    static const QStringList l {QStringLiteral("*.png")};

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

bool RmeUtils::hasPapaSmallPng(const QDir &dir)
{
    static QStringList l {QStringLiteral("*.png")};

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

RmeUtils::ExistNotes RmeUtils::existNotes(const QDir &dir)
{
    static const QMap<ExistNote, QString> suffixs {
        std::make_pair(IMD_4K_EZ, QStringLiteral("_4k_ez.imd")),    std::make_pair(IMD_4K_NM, QStringLiteral("_4k_nm.imd")),
        std::make_pair(IMD_4K_HD, QStringLiteral("_4k_hd.imd")),    std::make_pair(IMD_5K_EZ, QStringLiteral("_5k_ez.imd")),
        std::make_pair(IMD_5K_NM, QStringLiteral("_5k_nm.imd")),    std::make_pair(IMD_5K_HD, QStringLiteral("_5k_hd.imd")),
        std::make_pair(IMD_6K_EZ, QStringLiteral("_6k_ez.imd")),    std::make_pair(IMD_6K_NM, QStringLiteral("_6k_nm.imd")),
        std::make_pair(IMD_6K_HD, QStringLiteral("_6k_hd.imd")),    std::make_pair(MDE_EZ, QStringLiteral("_Papa_Easy.mde")),
        std::make_pair(MDE_NM, QStringLiteral("_Papa_Normal.mde")), std::make_pair(MDE_HD, QStringLiteral("_Papa_Hard.mde")),
    };

    ExistNotes result;
    for (ExistNote i = IMD_4K_EZ; i <= MDE_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(dir.dirName()).append(suffixs[i]);
        if (dir.exists(file_name))
            result |= i;
    }
    return result;
}

QString RmeUtils::calculateSongTime(int gameTime)
{
    double songTime = gameTime / 1440.;
    int songTimeInt = (int)songTime;

    QString r = QString::number(songTime, 'f', qMax(7 - QString::number(songTimeInt).length(), 0));

    r = r.left(8);
    return r;
}
