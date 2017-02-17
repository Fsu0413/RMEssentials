#include "unrarcliplugin.h"

#include <QDir>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

namespace {
bool israr(const QString &output)
{
    if (output.contains(QRegularExpression(QStringLiteral("\b(UN)?RAR\b"))))
        return true;
    return false;
}
}

RmeUncompUnrarCliPlugin::RmeUncompUnrarCliPlugin(QObject *parent)
    : QObject(parent)
{
}

bool RmeUncompUnrarCliPlugin::isReadyForUse() const
{
    return supportedFormats();
}

RmeUncompresserFormats RmeUncompUnrarCliPlugin::supportedFormats() const
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

    if (!israr(QString::fromLocal8Bit(process.readAll())))
        return RmeUncompresserFormats();

    return RmeUncompRar;
}

RmeUncompresserResult RmeUncompUnrarCliPlugin::openArchive(const QString &fileName)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (fileName.isEmpty())
        return RmeUncompFileNotExist;

    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("lb"), QStringLiteral("-p") + password(), fileName }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished()) {
        process.kill();
        return RmeUncompSubProcessError;
    }
    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1) {
        switch (exitCode) {
        case 3:
            return RmeUncompChecksumError;
        case 6:
            return RmeUncompFileNotExist;
        case 10:
            return RmeUncompUnknownFileFormat;
        case 11:
            return RmeUncompIncorrectPassword;
        default:
            return RmeUncompSubProcessError;
        }
    }

    m_fileName = fileName;
    return RmeUncompSuccess;
}

QStringList RmeUncompUnrarCliPlugin::listFiles()
{
    if (!isReadyForUse())
        return QStringList();

    if (m_fileName.isEmpty())
        return QStringList();

    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setReadChannel(QProcess::StandardOutput);
    process.start(binaryPath(), { QStringLiteral("lb"), QStringLiteral("-p") + password(), m_fileName }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished()) {
        process.kill();
        return QStringList();
    }
    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1)
        return QStringList();

    QStringList files;
    while (!process.atEnd()) {
        QString l = QString::fromLocal8Bit(process.readLine());
        if (!l.isEmpty())
            files << l;
    }

    return files;
}

RmeUncompresserResult RmeUncompUnrarCliPlugin::uncompressAllFiles(const QDir &targetDir)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (m_fileName.isEmpty())
        return RmeUncompFileNotExist;
    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("x"), QStringLiteral("-o+"), QStringLiteral("-y"), QStringLiteral("-p") + password(), m_fileName, QDir::toNativeSeparators(targetDir.absolutePath()) }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished(300000)) {
        process.kill();
        return RmeUncompSubProcessError;
    }

    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1) {
        switch (exitCode) {
        case 3:
            return RmeUncompChecksumError;
        case 6:
            return RmeUncompFileNotExist;
        case 10:
            return RmeUncompUnknownFileFormat;
        case 11:
            return RmeUncompIncorrectPassword;
        default:
            return RmeUncompSubProcessError;
        }
    }

    return RmeUncompSuccess;
}

RmeUncompresserResult RmeUncompUnrarCliPlugin::uncompressOneFile(const QDir &targetDir, const QString &fileName)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (fileName.isEmpty())
        return RmeUncompFileNotExist;
    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("e"), QStringLiteral("-o+"), QStringLiteral("-y"), QStringLiteral("-p") + password(), m_fileName, fileName, QDir::toNativeSeparators(targetDir.absolutePath()) }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished(300000)) {
        process.kill();
        return RmeUncompSubProcessError;
    }

    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1) {
        switch (exitCode) {
        case 3:
            return RmeUncompChecksumError;
        case 6:
            return RmeUncompFileNotExist;
        case 10:
            return RmeUncompUnknownFileFormat;
        case 11:
            return RmeUncompIncorrectPassword;
        default:
            return RmeUncompSubProcessError;
        }
    }
    return RmeUncompSuccess;
}

void RmeUncompUnrarCliPlugin::setPassword(const QString &password)
{
    m_password = password;
}

QJsonObject RmeUncompUnrarCliPlugin::pluginSettings() const
{
    QJsonObject ob;
    ob[QStringLiteral("binaryPath")] = binaryPath();
    return ob;
}

bool RmeUncompUnrarCliPlugin::setPluginSetting(const QString &key, const QJsonValue &value)
{
    if (key == QStringLiteral("binaryPath") && value.isString()) {
        m_binaryPath = value.toString();
        return true;
    }
    return false;
}

bool RmeUncompUnrarCliPlugin::setPluginSettings(const QJsonObject &json)
{
    bool result = true;
    foreach (const QString &key, json.keys())
        result &= setPluginSetting(key, json.value(key));
    return result;
}

QString RmeUncompUnrarCliPlugin::binaryPath() const
{
    if (m_binaryPath.isEmpty())
        return QStringLiteral("unrar");

    return m_binaryPath;
}

QString RmeUncompUnrarCliPlugin::password() const
{
    return m_password.isEmpty() ? QStringLiteral("-") : m_password;
}
