#include "renamer.h"
#include "utils.h"
#include <QDir>
#include <QString>
#include <QMap>


void Renamer::run()
{
    if (!m_d.exists()) {
        emit rename_finished(false);
        return;
    }

    if (m_toRename.isEmpty()) {
        emit rename_finished(false);
        return;
    }

    if (m_d.dirName() == m_toRename) {
        emit rename_finished(false);
        return;
    }

    QDir dotdotToRename(m_d.absolutePath() + "/../" + m_toRename);
    if (dotdotToRename.exists()) {
        emit rename_finished(false);
        return;
    }

    if (!hasMp3(m_d)) {
        emit rename_finished(false);
        return;
    }

    if (!hasBigPng(m_d)) {
        emit rename_finished(false);
        return;
    }

    if (!(renameMp3() && renameBigPng() && renameImds() && renameSmallPng() && renameSelf())) {
        emit rename_finished(false);
        return;
    }

    if (!deleteExtra()) {
        emit rename_finished(false);
        return;
    }

    emit rename_finished(true);
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
        return false;

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


bool Renamer::renameImds()
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


    for (ExistImd i = IMD_4K_EZ; i <= IMD_6K_HD; i = static_cast<ExistImd>(i << 1)) {
        QString file_name;
        file_name.append(m_d.dirName()).append(suffixs[i]).append(".imd");
        if (m_d.exists(file_name))
            m_d.rename(file_name, m_toRename + suffixs[i] + ".imd");
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
    foreach (const QString &s, m_d.entryList()) {
        if (!s.startsWith(m_d.dirName()))
            m_d.remove(s);

    }
    return true;
}
