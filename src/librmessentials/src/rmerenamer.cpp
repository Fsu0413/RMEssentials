#include "rmerenamer.h"
#include "rmeutils.h"

#include <QDir>
#include <QMap>
#include <QString>

using namespace RmeUtils;

class RmeRenamerPrivate
{
public:
    bool renameMp3();
    bool renameBigPng();
    bool renameSmallPng();
    bool renamePapaPngs();
    bool renameImds();
    bool renameImdsToEasy();
    bool renameSelf();
    bool deleteExtra();

    QDir m_dir;
    QString m_toRename;
};

RmeRenamer::RmeRenamer()
    : d_ptr(new RmeRenamerPrivate)
{
}

RmeRenamer::~RmeRenamer()
{
    delete d_ptr;
}

bool RmeRenamer::run()
{
    Q_D(RmeRenamer);
    if (!d->m_dir.exists())
        return false;

    if (d->m_toRename.isEmpty())
        return false;

    if (d->m_dir.dirName() == d->m_toRename)
        return false;

    QDir dotdotToRename(d->m_dir.absolutePath() + QStringLiteral("/../") + d->m_toRename);
    if (dotdotToRename.exists())
        return false;

    if (!hasMp3(d->m_dir))
        return false;

    if (!hasBigPng(d->m_dir))
        return false;

    if (!(d->renameMp3() && d->renameBigPng() && d->renameImds() && d->renameSmallPng() && d->renamePapaPngs() && d->renameSelf()))
        return false;

    if (!d->deleteExtra())
        return false;

    return true;
}

bool RmeRenamer::runToEasy()
{
    Q_D(RmeRenamer);
    if (!d->m_dir.exists())
        return false;

    if (!d->renameImdsToEasy())
        return false;

    return true;
}

bool RmeRenamerPrivate::renameImdsToEasy()
{
    for (ExistNote i = IMD_4K_EZ; i <= MDE_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(m_dir.dirName()).append(noteFileNameSuffix(i));
        if (m_dir.exists(file_name)) {
            ExistNote i_easiest = i;
            while (!noteFileNameSuffix(i_easiest).contains(QStringLiteral("ez")) && !noteFileNameSuffix(i_easiest).contains(QStringLiteral("Easy")))
                i_easiest = static_cast<ExistNote>(i_easiest >> 1);

            while (m_dir.exists(m_dir.dirName() + noteFileNameSuffix(i_easiest)) && (m_dir.dirName() + noteFileNameSuffix(i_easiest)) != file_name)
                i_easiest = static_cast<ExistNote>(i_easiest << 1);

            if ((m_dir.dirName() + noteFileNameSuffix(i_easiest)) != file_name)
                m_dir.rename(file_name, (m_dir.dirName() + noteFileNameSuffix(i_easiest)));
        }
    }

    return true;
}

bool RmeRenamerPrivate::renameMp3()
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.mp3");

    QString origname;
    foreach (const QString &s, m_dir.entryList(l)) {
        if (s.toLower() == (m_dir.dirName() + QStringLiteral(".mp3")).toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return false;

    m_dir.rename(origname, m_toRename + QStringLiteral(".mp3"));
    return true;
}

bool RmeRenamerPrivate::renameBigPng()
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    QString origname;
    foreach (const QString &s, m_dir.entryList(l)) {
        if (s.toLower() == (m_dir.dirName() + QStringLiteral(".png")).toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return true; // in fact the Bluecat 3 version 2.1 can play imds without big pngs, so ignore this

    m_dir.rename(origname, m_toRename + QStringLiteral(".png"));
    return true;
}

bool RmeRenamerPrivate::renameSmallPng()
{
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    QString suffix;
    if (!hasSmallPng(m_dir, suffix))
        return true; // in fact we can play this imd without small pngs in Bluecat 3, so temporily ignore this, I will make a setting later to set this

    QString origname;
    foreach (const QString &s, m_dir.entryList(l)) {
        if (s.toLower() == (m_dir.dirName() + suffix + QStringLiteral(".png")).toLower()) {
            origname = s;
            break;
        }
    }

    if (origname.isEmpty())
        return true; // also ignore

    m_dir.rename(origname, m_toRename + QStringLiteral("_title_ipad.png"));
    // QFile::copy(m_dir.absoluteFilePath(m_toRename + "_title_ipad.png"), m_dir.absoluteFilePath(m_toRename + "_title_hd.png"));
    return true;
}

bool RmeRenamerPrivate::renamePapaPngs()
{
    // in fact, not all notes has Papa mode, so ignore the result here
    static QStringList l;
    if (l.isEmpty())
        l << QStringLiteral("*.png");

    QString origname_small;
    QString origname_big;
    foreach (const QString &s, m_dir.entryList(l)) {
        if (s.toLower() == (m_dir.dirName() + QStringLiteral("_title_140_90.png")).toLower()) {
            origname_small = s;
            break;
        } else if (s.toLower() == (m_dir.dirName() + QStringLiteral("_ipad.png")).toLower()) {
            origname_big = s;
        }
    }

    if (!origname_small.isEmpty())
        m_dir.rename(origname_small, m_toRename + QStringLiteral("_title_140_90.png"));
    if (!origname_big.isEmpty())
        m_dir.rename(origname_big, m_toRename + QStringLiteral("_ipad.png"));

    return true;
}

bool RmeRenamerPrivate::renameImds()
{
    for (ExistNote i = IMD_4K_EZ; i <= MDE_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(m_dir.dirName()).append(noteFileNameSuffix(i));
        if (m_dir.exists(file_name))
            m_dir.rename(file_name, m_toRename + noteFileNameSuffix(i));
    }

    return true;
}

bool RmeRenamerPrivate::renameSelf()
{
    QString oldName = m_dir.dirName();
    m_dir.cdUp();
    m_dir.rename(oldName, m_toRename);
    m_dir.cd(m_toRename);
    return m_dir.exists();
}

bool RmeRenamerPrivate::deleteExtra()
{
    foreach (const QFileInfo &s, m_dir.entryInfoList()) {
        if (s.isDir()) {
            QDir d(s.absoluteFilePath());
            d.cdUp();
            if (d != m_dir)
                continue;
            QDir(s.absoluteFilePath()).removeRecursively();
        } else {
            QDir d(s.path());
            if (d != m_dir)
                continue;
            if (!s.fileName().toLower().startsWith(m_dir.dirName().toLower()))
                m_dir.remove(s.absoluteFilePath());
        }
    }
    return true;
}

void RmeRenamer::setDir(const QDir &dir)
{
    Q_D(RmeRenamer);
    d->m_dir = dir;
}

const QDir &RmeRenamer::dir() const
{
    Q_D(const RmeRenamer);
    return d->m_dir;
}

void RmeRenamer::setToRename(const QString &to)
{
    Q_D(RmeRenamer);
    d->m_toRename = to;
}

const QString &RmeRenamer::toRename() const
{
    Q_D(const RmeRenamer);
    return d->m_toRename;
}
