#include "quazipplugin.h"

#include <QDir>
#include <QJsonObject>

#include "quazip.h"
#include "quazipfile.h"

RmeUncompLibQuaZipPlugin::RmeUncompLibQuaZipPlugin(QObject *parent)
    : RmeUncompresserPlugin(parent)
    , m_zip(nullptr)
{
}

bool RmeUncompLibQuaZipPlugin::isReadyForUse() const
{
    return true;
}

RmeUncompresserFormats RmeUncompLibQuaZipPlugin::supportedFormats() const
{
    return RmeUncompZip;
}

RmeUncompresserResult RmeUncompLibQuaZipPlugin::openArchive(const QString &fileName)
{
    if (m_zip != nullptr)
        delete m_zip;

    m_zip = new QuaZip(fileName);
    if (!m_zip->open(QuaZip::mdUnzip)) {
        delete m_zip;
        m_zip = nullptr;
        return RmeUncompUnknownError;
    }

    m_fileName = fileName;
    return RmeUncompSuccess;
}

QStringList RmeUncompLibQuaZipPlugin::listFiles()
{
    if (m_zip == nullptr)
        return QStringList();

    return m_zip->getFileNameList();
}

RmeUncompresserResult RmeUncompLibQuaZipPlugin::uncompressAllFiles(const QDir &targetDir)
{
    if (m_zip == nullptr)
        return RmeUncompFileNotExist;

    QStringList fileList = m_zip->getFileNameList();
    RmeUncompresserResult r = RmeUncompSuccess;

    foreach (const QString &item, fileList) {
        RmeUncompresserResult ritem = uncompressOneFile(targetDir, item);
        if (ritem != RmeUncompSuccess)
            r = ritem;
    }

    return r;
}

RmeUncompresserResult RmeUncompLibQuaZipPlugin::uncompressOneFile(const QDir &targetDir, const QString &fileName)
{
    if (m_zip == nullptr)
        return RmeUncompFileNotExist;

    m_zip->setCurrentFile(fileName);
    QuaZipFile file(m_zip);
    if (file.open(QIODevice::ReadOnly, password().toLocal8Bit().constData())) {
        QString s = targetDir.absoluteFilePath(fileName);
        QDir().mkpath(QFileInfo(s).absolutePath());
        QFile f(s);
        f.open(QIODevice::WriteOnly | QIODevice::Truncate);
        f.write(file.readAll());
        f.close();
        file.close();
    } else
        return RmeUncompFileNotExist;

    return RmeUncompSuccess;
}
