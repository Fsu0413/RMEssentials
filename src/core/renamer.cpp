#include "renamer.h"
#include "utils.h"
#include <QDir>
#include <QString>
#include <QMap>


bool Renamer::run()
{
    if (!m_d.exists())
        return false;

    if (m_toRename.isEmpty())
        return false;

    if (m_d.dirName() == m_toRename)
        return false;

    QDir dotdotToRename(m_d.absolutePath() + "/../" + m_toRename);
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
        suffixs[IMD_4K_EZ] = "_4k_ez.imd";
        suffixs[IMD_4K_NM] = "_4k_nm.imd";
        suffixs[IMD_4K_HD] = "_4k_hd.imd";
        suffixs[IMD_5K_EZ] = "_5k_ez.imd";
        suffixs[IMD_5K_NM] = "_5k_nm.imd";
        suffixs[IMD_5K_HD] = "_5k_hd.imd";
        suffixs[IMD_6K_EZ] = "_6k_ez.imd";
        suffixs[IMD_6K_NM] = "_6k_nm.imd";
        suffixs[IMD_6K_HD] = "_6k_hd.imd";
        suffixs[MDE_EZ] = "_Papa_Easy.mde";
        suffixs[MDE_NM] = "_Papa_Normal.mde";
        suffixs[MDE_HD] = "_Papa_Hard.mde";
    }

    for (ExistNote i = IMD_4K_EZ; i <= MDE_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(m_d.dirName()).append(suffixs[i]);
        if (m_d.exists(file_name)) {
            ExistNote i_easiest = i;
            while (!suffixs[i_easiest].contains("ez") && !suffixs[i_easiest].contains("Easy"))
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
        l << "*.mp3";

    QString origname;
    foreach (const QString &s, m_d.entryList(l)) {
        if (s.toLower() == (m_d.dirName() + ".mp3").toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return false;

    m_d.rename(origname, m_toRename + ".mp3");
    return true;
}

bool Renamer::renameBigPng()
{
    static QStringList l;
    if (l.isEmpty())
        l << "*.png";

    QString origname;
    foreach (const QString &s, m_d.entryList(l)) {
        if (s.toLower() == (m_d.dirName() + ".png").toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return true; // in fact the Bluecat 3 version 2.1 can play imds without big pngs, so ignore this

    m_d.rename(origname, m_toRename + ".png");
    return true;
}

bool Renamer::renameSmallPng()
{
    static QStringList l;
    if (l.isEmpty())
        l << "*.png";

    const char *suffix = NULL;
    if (!hasSmallPng(m_d, suffix))
        return true; // in fact we can play this imd without small pngs, so ignore this

    QString origname;
    foreach (const QString &s, m_d.entryList(l)) {
        if (s.toLower() == (m_d.dirName() + QString(suffix) + ".png").toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return true; // also ignore

    m_d.rename(origname, m_toRename + "_title_ipad.png");
    return true;
}

bool Renamer::renamePapaPngs()
{
    // in fact, not all notes has Papa mode, so ignore the result here
    static QStringList l;
    if (l.isEmpty())
        l << "*.png";

    QString origname_small;
    QString origname_big;
    foreach (const QString &s, m_d.entryList(l)) {
        if (s.toLower() == (m_d.dirName() + "_title_140_90.png").toLower()) {
            origname_small = s;
            break;
        } else if (s.toLower() == (m_d.dirName() + "_ipad.png").toLower()) {
            origname_big = s;
        }
    }

    if (!origname_small.isEmpty())
        m_d.rename(origname_small, m_toRename + "_title_140_90.png");
    if (!origname_big.isEmpty())
        m_d.rename(origname_big, m_toRename + "_ipad.png");

    return true;
}

bool Renamer::renameImds()
{
    static QMap<ExistNote, QString> suffixs;
    if (suffixs.isEmpty()) {
        suffixs[IMD_4K_EZ] = "_4k_ez.imd";
        suffixs[IMD_4K_NM] = "_4k_nm.imd";
        suffixs[IMD_4K_HD] = "_4k_hd.imd";
        suffixs[IMD_5K_EZ] = "_5k_ez.imd";
        suffixs[IMD_5K_NM] = "_5k_nm.imd";
        suffixs[IMD_5K_HD] = "_5k_hd.imd";
        suffixs[IMD_6K_EZ] = "_6k_ez.imd";
        suffixs[IMD_6K_NM] = "_6k_nm.imd";
        suffixs[IMD_6K_HD] = "_6k_hd.imd";
        suffixs[MDE_EZ] = "_Papa_Easy.mde";
        suffixs[MDE_NM] = "_Papa_Normal.mde";
        suffixs[MDE_HD] = "_Papa_Hard.mde";
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
