#include "unzipcliplugin.h"

#include <QDir>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

namespace {
bool iszip(const QString &output)
{
    if (output.contains(QRegularExpression(QStringLiteral("\bInfo-ZIP\b"))))
        return true;
    return false;
}
}

RmeUncompUnzipCliPlugin::RmeUncompUnzipCliPlugin(QObject *parent)
    : RmeUncompresserPlugin(parent)
{
}

bool RmeUncompUnzipCliPlugin::isReadyForUse() const
{
    return supportedFormats();
}

RmeUncompresserFormats RmeUncompUnzipCliPlugin::supportedFormats() const
{
    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setReadChannel(QProcess::StandardOutput);
    process.start(binaryPath(), QStringList(), QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished()) {
        process.kill();
        return RmeUncompresserFormats();
    }
    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1)
        return RmeUncompresserFormats();

    if (!iszip(QString::fromLocal8Bit(process.readAll())))
        return RmeUncompresserFormats();

    return RmeUncompZip;
}

RmeUncompresserResult RmeUncompUnzipCliPlugin::openArchive(const QString &fileName)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (fileName.isEmpty())
        return RmeUncompFileNotExist;

    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("-Z1"), fileName }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished()) {
        process.kill();
        return RmeUncompSubProcessError;
    }
    int exitCode = process.exitCode();
    if (exitCode != 0)
        return RmeUncompSubProcessError;

    m_fileName = fileName;
    return RmeUncompSuccess;
}

QStringList RmeUncompUnzipCliPlugin::listFiles()
{
    if (!isReadyForUse())
        return QStringList();

    if (m_fileName.isEmpty())
        return QStringList();

    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setReadChannel(QProcess::StandardOutput);
    process.start(binaryPath(), { QStringLiteral("-Z1"), m_fileName }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished()) {
        process.kill();
        return QStringList();
    }
    int exitCode = process.exitCode();
    if (exitCode != 0)
        return QStringList();

    QStringList files;
    while (!process.atEnd()) {
        QString l = QString::fromLocal8Bit(process.readLine());
        if (!l.isEmpty())
            files << l;
    }

    return files;
}

RmeUncompresserResult RmeUncompUnzipCliPlugin::uncompressAllFiles(const QDir &targetDir)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (m_fileName.isEmpty())
        return RmeUncompFileNotExist;
    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("-P"), password(), QStringLiteral("-o"), m_fileName, QStringLiteral("-d"), QDir::toNativeSeparators(targetDir.absolutePath()) }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished(300000)) {
        process.kill();
        return RmeUncompSubProcessError;
    }

    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1) {
        switch (exitCode) {
        case 9:
            return RmeUncompFileNotExist;
        case 81:
            return RmeUncompUnknownFileFormat;
        case 82:
            return RmeUncompIncorrectPassword;
        default:
            return RmeUncompSubProcessError;
        }
    }

    return RmeUncompSuccess;
}

RmeUncompresserResult RmeUncompUnzipCliPlugin::uncompressOneFile(const QDir &targetDir, const QString &fileName)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (fileName.isEmpty())
        return RmeUncompFileNotExist;
    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("-j"), QStringLiteral("-P"), password(), QStringLiteral("-o"), m_fileName, fileName, QStringLiteral("-d"), QDir::toNativeSeparators(targetDir.absolutePath()) }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished(300000)) {
        process.kill();
        return RmeUncompSubProcessError;
    }

    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1) {
        switch (exitCode) {
        case 9:
            return RmeUncompFileNotExist;
        case 81:
            return RmeUncompUnknownFileFormat;
        case 82:
            return RmeUncompIncorrectPassword;
        default:
            return RmeUncompSubProcessError;
        }
    }

    return RmeUncompSuccess;
}

QString RmeUncompUnzipCliPlugin::binaryPath() const
{
    return m_binaryPath.isEmpty() ? QStringLiteral("unzip") : m_binaryPath;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
QVersionNumber RmeUncompUnzipCliPlugin::versionNumber() const
{
    return QVersionNumber();
}
#endif

const char *RmeUncompUnzipCliPlugin::version() const
{
    return "0";
}
