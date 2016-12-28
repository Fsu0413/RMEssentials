#include "renamer.h"
#include "utils.h"

#include <QMap>
#include <QString>

bool Renamer::run()
{
    if (!m_d.exists())
        return false;

    if (m_toRename.isEmpty())
        return false;

    if (m_d.dirName() == m_toRename)
        return false;

    QDir dotdotToRename(m_d.absolutePath() + QStringLiteral("/../") + m_toRename);
    if (dotdotToRename.exists())
        return false;

    if (!hasMp3(m_d))
        return false;

    if (!hasBigPng(m_d))
        return false;

    if (!(renameMp3() && renameBigPng() && renameImds() && renameSmallPng() && renamePapaPngs() && renameSelf()))
        return false;

    if (!deleteExtra())
        return false;

    return true;
}

bool Renamer::runToEasy()
{
    if (!m_d.exists())
        return false;

    if (!renameImdsToEasy())
        return false;

    return true;
}

bool Renamer::renameImdsToEasy()
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

    for (ExistNote i = IMD_4K_EZ; i <= MDE_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(m_d.dirName()).append(suffixs[i]);
        if (m_d.exists(file_name)) {
            ExistNote i_easiest = i;
            while (!suffixs[i_easiest].contains(QStringLiteral("ez")) && !suffixs[i_easiest].contains(QStringLiteral("Easy")))
                i_easiest = static_cast<ExistNote>(i_easiest >> 1);

            while (m_d.exists(m_d.dirName() + suffixs[i_easiest]) && (m_d.dirName() + suffixs[i_easiest]) != file_name)
                i_easiest = static_cast<ExistNote>(i_easiest << 1);

            if ((m_d.dirName() + suffixs[i_easiest]) != file_name)
                m_d.rename(file_name, (m_d.dirName() + suffixs[i_easiest]));
        }
    }

    return true;
}

bool Renamer::renameMp3()
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.mp3");

    QString origname;
    foreach (const QString &s, m_d.entryList(l)) {
        if (s.toLower() == (m_d.dirName() + QStringLiteral(".mp3")).toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return false;

    m_d.rename(origname, m_toRename + QStringLiteral(".mp3"));
    return true;
}

bool Renamer::renameBigPng()
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    QString origname;
    foreach (const QString &s, m_d.entryList(l)) {
        if (s.toLower() == (m_d.dirName() + QStringLiteral(".png")).toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return true; // in fact the Bluecat 3 version 2.1 can play imds without big pngs, so ignore this

    m_d.rename(origname, m_toRename + QStringLiteral(".png"));
    return true;
}

bool Renamer::renameSmallPng()
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    QString suffix;
    if (!hasSmallPng(m_d, suffix))
        return true; // in fact we can play this imd without small pngs in Bluecat 3, so temporily ignore this, I will make a setting later to set this

    QString origname;
    foreach (const QString &s, m_d.entryList(l)) {
        if (s.toLower() == (m_d.dirName() + suffix + QStringLiteral(".png")).toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return true; // also ignore

    m_d.rename(origname, m_toRename + QStringLiteral("_title_ipad.png"));
    // QFile::copy(m_d.absoluteFilePath(m_toRename + "_title_ipad.png"), m_d.absoluteFilePath(m_toRename + "_title_hd.png")); // I don't know whether the small pngs is in this style of name in iOS version
    return true;
}

bool Renamer::renamePapaPngs()
{
    // in fact, not all notes has Papa mode, so ignore the result here
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    QString origname_small;
    QString origname_big;
    foreach (const QString &s, m_d.entryList(l)) {
        if (s.toLower() == (m_d.dirName() + QStringLiteral("_title_140_90.png")).toLower()) {
            origname_small = s;
            break;
        } else if (s.toLower() == (m_d.dirName() + QStringLiteral("_ipad.png")).toLower()) {
            origname_big = s;
        }
    }

    if (!origname_small.isEmpty())
        m_d.rename(origname_small, m_toRename + QStringLiteral("_title_140_90.png"));
    if (!origname_big.isEmpty())
        m_d.rename(origname_big, m_toRename + QStringLiteral("_ipad.png"));

    return true;
}

bool Renamer::renameImds()
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

    for (ExistNote i = IMD_4K_EZ; i <= MDE_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(m_d.dirName()).append(suffixs[i]);
        if (m_d.exists(file_name))
            m_d.rename(file_name, m_toRename + suffixs[i]);
    }

    return true;
}

bool Renamer::renameSelf()
{
    QString oldName = m_d.dirName();
    m_d.cdUp();
    m_d.rename(oldName, m_toRename);
    m_d.cd(m_toRename);
    return m_d.exists();
}

bool Renamer::deleteExtra()
{
    foreach (const QFileInfo &s, m_d.entryInfoList()) {
        if (s.isDir()) {
            QDir d(s.absoluteFilePath());
            d.cdUp();
            if (d != m_d)
                continue;
            QDir(s.absoluteFilePath()).removeRecursively();
        } else {
            QDir d(s.path());
            if (d != m_d)
                continue;
            if (!s.fileName().toLower().startsWith(m_d.dirName().toLower()))
                m_d.remove(s.absoluteFilePath());
        }
    }
    return true;
}
