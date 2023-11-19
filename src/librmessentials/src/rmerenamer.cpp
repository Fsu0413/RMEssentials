#include "rmerenamer.h"
#include "rmechart.h"
#include "rmecrypt.h"
#include "rmeutils.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QString>

using namespace RmeUtils;

class RmeRenamerPrivate
{
public:
    bool renameSingleRmp(const QString &oldName, const QString &newName);

    bool renameMp3();
    bool renamePngs();
    bool renamePatterns();
    bool renameImds();
    bool renameImdJsons();
    bool renameRmps();
    bool renamePatternsToEasy();
    bool renameImdsToEasy();
    bool renameImdJsonsToEasy();
    bool renameRmpsToEasy();
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

    QDir dotdotToRename = d->m_dir;
    if (dotdotToRename.cdUp() && dotdotToRename.exists())
        return false;

    if (!hasMp3(d->m_dir))
        return false;

    if (!hasBigPng(d->m_dir))
        return false;

    if (!(d->renameMp3() && d->renamePngs() && d->renamePatterns() && d->renameSelf()))
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

    if (!d->renamePatternsToEasy())
        return false;

    return true;
}

bool RmeRenamerPrivate::renameSingleRmp(const QString &oldName, const QString &newName)
{
    QFileInfo fi(m_dir.absoluteFilePath(oldName));
    QFile f(fi.absolutePath());
    if (!f.open(QFile::ReadOnly))
        return false;

    QByteArray arr = f.readAll();
    f.close();

    QByteArray rmpKeyOld = RmeUtils::rmpKeyForChart(fi.baseName());
    rmpKeyOld.resize(16);
    QByteArray decryptedArr = RmeCrypt::decryptXxteaOnly(arr, rmpKeyOld);

    QFileInfo fiNew(m_dir.absoluteFilePath(newName));
    QByteArray rmpKeyNew = RmeUtils ::rmpKeyForChart(fiNew.baseName());
    rmpKeyNew.resize(16);
    QByteArray encryptedArr = RmeCrypt::encryptXxteaOnly(decryptedArr, rmpKeyNew);

    QFile fNew(fiNew.absolutePath());
    if (!fNew.open(QFile::WriteOnly | QFile::Truncate))
        return false;
    fNew.write(encryptedArr);
    fNew.close();

    m_dir.remove(oldName);

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

bool RmeRenamerPrivate::renameImdJsonsToEasy()
{
    for (ExistNote i = IMDJSON_4K_EZ; i <= IMDJSON_6K_HD; i = static_cast<ExistNote>(i << 1)) {
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

bool RmeRenamerPrivate::renameRmpsToEasy()
{
    for (ExistNote i = RMP_4K_EZ; i <= RMP_6K_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(m_dir.dirName()).append(noteFileNameSuffix(i));
        if (m_dir.exists(file_name)) {
            ExistNote i_easiest = i;
            while (!noteFileNameSuffix(i_easiest).contains(QStringLiteral("ez")) && !noteFileNameSuffix(i_easiest).contains(QStringLiteral("Easy")))
                i_easiest = static_cast<ExistNote>(i_easiest >> 1);

            while (m_dir.exists(m_dir.dirName() + noteFileNameSuffix(i_easiest)) && (m_dir.dirName() + noteFileNameSuffix(i_easiest)) != file_name)
                i_easiest = static_cast<ExistNote>(i_easiest << 1);

            if ((m_dir.dirName() + noteFileNameSuffix(i_easiest)) != file_name)
                renameSingleRmp(file_name, (m_dir.dirName() + noteFileNameSuffix(i_easiest)));
        }
    }

    return true;
}

bool RmeRenamerPrivate::renameMp3()
{
    static const QStringList l {QStringLiteral("*.mp3")};

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

bool RmeRenamerPrivate::renamePngs()
{
    static QStringList l {QStringLiteral("*.png")};
    // clang-format off
    static QStringList suffixes {
        QLatin1String(""),
        QStringLiteral("_title_hd"),
        QStringLiteral("_title_ipad"),
        QStringLiteral("_ipad"),
        QStringLiteral("_thumb"),
        QStringLiteral("_title_140_90"),
    };
    // clang-format on

    QList<std::pair<QString /* origName */, QString /* toRename */> > renameList;

    foreach (const QString &s, m_dir.entryList(l)) {
        foreach (const QString &suffix, suffixes) {
            if (s.toLower() == (m_dir.dirName() + suffix + QStringLiteral(".png")).toLower()) {
                QString renameSuffix = suffix;
                if (renameSuffix == QStringLiteral("_title_hd"))
                    renameSuffix = QStringLiteral("_title_ipad");

                renameList << std::make_pair(s, m_toRename + suffix + QStringLiteral(".png"));
                break;
            }
        }
    }

    if (renameList.isEmpty())
        return true; // in fact the Bluecat 3 version 2.1 can play imds without any PNGs, so ignore this

    // !! foreach is macro so type can't contain comma in it. 'auto' is used here with the type listed !!
    foreach (const auto /* std::pair<QString, QString> */ &p, renameList) {
        const auto &[/* QString */ origName, /* QString */ toRename] = p;
        m_dir.rename(origName, toRename);
    }

    return true;
}

bool RmeRenamerPrivate::renamePatterns()
{
    bool i = renameImds();
    bool ij = renameImdJsons();
    bool r = renameRmps();

    return i || ij || r;
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

bool RmeRenamerPrivate::renameImdJsons()
{
    for (ExistNote i = IMDJSON_4K_EZ; i <= IMDJSON_6K_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(m_dir.dirName()).append(noteFileNameSuffix(i));
        if (m_dir.exists(file_name))
            m_dir.rename(file_name, m_toRename + noteFileNameSuffix(i));
    }

    return true;
}

bool RmeRenamerPrivate::renameRmps()
{
    for (ExistNote i = RMP_4K_EZ; i <= RMP_6K_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(m_dir.dirName()).append(noteFileNameSuffix(i));
        if (m_dir.exists(file_name))
            renameSingleRmp(file_name, m_toRename + noteFileNameSuffix(i));
    }

    return true;
}

bool RmeRenamerPrivate::renamePatternsToEasy()
{
    bool i = renameImdsToEasy();
    bool ij = renameImdJsonsToEasy();
    bool r = renameRmpsToEasy();

    return i || ij || r;
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

class RmeConverterPrivate
{
public:
    QDir m_dir;
};

RmeConverter::RmeConverter()
    : d_ptr(new RmeConverterPrivate)
{
}

RmeConverter::~RmeConverter()
{
    delete d_ptr;
}

bool RmeConverter::convertImdToImdJson(const RmeChartVersion &version)
{
    Q_D(RmeConverter);
    if (!d->m_dir.exists())
        return false;

    bool flag = true;

    for (ExistNote i = IMD_4K_EZ; i <= IMD_6K_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(d->m_dir.dirName()).append(noteFileNameSuffix(i));
        if (d->m_dir.exists(file_name)) {
            QFile f(d->m_dir.absoluteFilePath(file_name));
            if (!f.open(QIODevice::ReadOnly)) {
                flag = false;
                continue;
            }

            QByteArray arr = f.readAll();
            f.close();

            bool ok = false;
            RmeChart c = RmeChart::fromImd(arr, &ok);
            if (!ok) {
                flag = false;
                continue;
            }
            QJsonObject ob = c.toJson(version);
            QJsonDocument doc(ob);
            QByteArray jsonArr = doc.toJson();
            ExistNote converted = static_cast<ExistNote>(i << 12);
            QString toFileName;
            toFileName.append(d->m_dir.dirName()).append(noteFileNameSuffix(converted));
            QFile fJson(d->m_dir.absoluteFilePath(toFileName));
            if (!fJson.open(QFile::WriteOnly)) {
                flag = false;
                continue;
            }
            fJson.write(jsonArr);
            fJson.close();
        }
    }

    return flag;
}

bool RmeConverter::convertImdJsonToImd()
{
    Q_D(RmeConverter);
    if (!d->m_dir.exists())
        return false;

    bool flag = true;

    for (ExistNote i = IMDJSON_4K_EZ; i <= IMDJSON_6K_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(d->m_dir.dirName()).append(noteFileNameSuffix(i));
        if (d->m_dir.exists(file_name)) {
            QFile f(d->m_dir.absoluteFilePath(file_name));
            if (!f.open(QIODevice::ReadOnly)) {
                flag = false;
                continue;
            }

            QByteArray arr = f.readAll();
            f.close();
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(arr, &err);
            if (err.error != QJsonParseError::NoError) {
                flag = false;
                continue;
            }
            if (!doc.isObject()) {
                flag = false;
                continue;
            }
            QJsonObject ob = doc.object();
            bool ok = false;
            RmeChart c = RmeChart::fromJson(ob, &ok);
            if (!ok) {
                flag = false;
                continue;
            }
            QByteArray arrImd = c.toImd();
            ExistNote converted = static_cast<ExistNote>(i >> 12);
            QString toFileName;
            toFileName.append(d->m_dir.dirName()).append(noteFileNameSuffix(converted));
            QFile fImd(d->m_dir.absoluteFilePath(toFileName));
            if (!fImd.open(QFile::WriteOnly)) {
                flag = false;
                continue;
            }
            fImd.write(arrImd);
            fImd.close();
        }
    }

    return flag;
}

bool RmeConverter::convertRmpToImdJson()
{
    Q_D(RmeConverter);
    if (!d->m_dir.exists())
        return false;

    bool flag = true;

    for (ExistNote i = RMP_4K_EZ; i <= RMP_6K_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(d->m_dir.dirName()).append(noteFileNameSuffix(i));
        if (d->m_dir.exists(file_name)) {
            QFile f(d->m_dir.absoluteFilePath(file_name));
            if (!f.open(QIODevice::ReadOnly)) {
                flag = false;
                continue;
            }

            QByteArray arr = f.readAll();
            f.close();

            ExistNote converted = static_cast<ExistNote>(i >> 9);
            QString toFileName;
            toFileName.append(d->m_dir.dirName()).append(noteFileNameSuffix(converted));

            QByteArray rmpKey = RmeUtils::rmpKeyForChart(file_name);
            QByteArray imdjson = RmeCrypt::decryptFull(arr, rmpKey);

            QFile fImdJson(d->m_dir.absoluteFilePath(toFileName));
            if (!fImdJson.open(QFile::WriteOnly)) {
                flag = false;
                continue;
            }
            fImdJson.write(imdjson);
            fImdJson.close();
        }
    }

    return flag;
}

bool RmeConverter::convertImdJsonToRmp()
{
    Q_D(RmeConverter);
    if (!d->m_dir.exists())
        return false;

    bool flag = true;

    for (ExistNote i = IMDJSON_4K_EZ; i <= IMDJSON_6K_HD; i = static_cast<ExistNote>(i << 1)) {
        QString file_name;
        file_name.append(d->m_dir.dirName()).append(noteFileNameSuffix(i));
        if (d->m_dir.exists(file_name)) {
            QFile f(d->m_dir.absoluteFilePath(file_name));
            if (!f.open(QIODevice::ReadOnly)) {
                flag = false;
                continue;
            }

            QByteArray arr = f.readAll();
            f.close();

            ExistNote converted = static_cast<ExistNote>(i << 9);
            QString toFileName;
            toFileName.append(d->m_dir.dirName()).append(noteFileNameSuffix(converted));

            QByteArray rmpKey = RmeUtils::rmpKeyForChart(toFileName);
            QByteArray rmp = RmeCrypt::encryptFull(arr, rmpKey);

            QFile fRmp(d->m_dir.absoluteFilePath(toFileName));
            if (!fRmp.open(QFile::WriteOnly)) {
                flag = false;
                continue;
            }
            fRmp.write(rmp);
            fRmp.close();
        }
    }

    return flag;
}

void RmeConverter::setDir(const QDir &dir)
{
    Q_D(RmeConverter);
    d->m_dir = dir;
}

const QDir &RmeConverter::dir() const
{
    Q_D(const RmeConverter);
    return d->m_dir;
}
