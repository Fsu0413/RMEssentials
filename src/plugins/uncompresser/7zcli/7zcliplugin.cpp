#include "7zcliplugin.h"

#include <QDir>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

namespace {
bool is7z(const QString &output)
{
    if (output.contains(QRegularExpression(QStringLiteral("\b7-Zip\b"))))
        return true;
    return false;
}
}

RmeUncomp7zCliPlugin::RmeUncomp7zCliPlugin(QObject *parent)
    : QObject(parent)
{
}

bool RmeUncomp7zCliPlugin::isReadyForUse() const
{
    return supportedFormats();
}

RmeUncompresserFormats RmeUncomp7zCliPlugin::supportedFormats() const
{
    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setReadChannel(QProcess::StandardOutput);
    process.start(binaryPath(), { QStringLiteral("i") }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished()) {
        process.kill();
        return RmeUncompresserFormats();
    }

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    if (!is7z(output))
        return RmeUncompresserFormats();

    RmeUncompresserFormats f = RmeUncomp7z;

    QRegularExpression z(QStringLiteral("\\bzip\\b"));
    QRegularExpressionMatch zm = z.match(output);
    if (zm.hasMatch())
        f |= RmeUncompZip;

    QRegularExpression r(QStringLiteral("\\brar\\b"));
    QRegularExpressionMatch rm = r.match(output);
    if (rm.hasMatch())
        f |= RmeUncompRar;

    return f;
}

RmeUncompresserResult RmeUncomp7zCliPlugin::openArchive(const QString &fileName)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (fileName.isEmpty())
        return RmeUncompFileNotExist;

    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("l"), QStringLiteral("-p") + password(), QStringLiteral("-ba"), fileName }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished()) {
        process.kill();
        return RmeUncompSubProcessError;
    }
    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1)
        return RmeUncompSubProcessError;

    m_fileName = fileName;
    return RmeUncompSuccess;
}

QStringList RmeUncomp7zCliPlugin::listFiles()
{
    if (!isReadyForUse())
        return QStringList();

    if (m_fileName.isEmpty())
        return QStringList();

    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setReadChannel(QProcess::StandardOutput);
    process.start(binaryPath(), { QStringLiteral("l"), QStringLiteral("-p") + password(), QStringLiteral("-ba"), m_fileName }, QIODevice::ReadOnly);
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
        QStringList sl = l.split(QRegularExpression(QStringLiteral("\\s")), QString::SkipEmptyParts);
        if (!sl.isEmpty())
            files << sl.last();
    }

    return files;
}

RmeUncompresserResult RmeUncomp7zCliPlugin::uncompressAllFiles(const QDir &targetDir)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (m_fileName.isEmpty())
        return RmeUncompFileNotExist;
    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("x"), QStringLiteral("-p") + password(), QStringLiteral("-o") + QDir::toNativeSeparators(targetDir.absolutePath()), m_fileName }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished(300000)) {
        process.kill();
        return RmeUncompSubProcessError;
    }

    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1)
        return RmeUncompSubProcessError;

    return RmeUncompSuccess;
}

RmeUncompresserResult RmeUncomp7zCliPlugin::uncompressOneFile(const QDir &targetDir, const QString &fileName)
{
    if (!isReadyForUse())
        return RmeUncompNotReadyForUse;

    if (fileName.isEmpty())
        return RmeUncompFileNotExist;
    QProcess process;
    process.setStandardErrorFile(QProcess::nullDevice());
    process.setStandardInputFile(QProcess::nullDevice());
    process.setStandardOutputFile(QProcess::nullDevice());

    process.start(binaryPath(), { QStringLiteral("e"), QStringLiteral("-p") + password(), QStringLiteral("-o") + QDir::toNativeSeparators(targetDir.absolutePath()), m_fileName, fileName }, QIODevice::ReadOnly);
    if (!process.waitForStarted() || !process.waitForFinished(300000)) {
        process.kill();
        return RmeUncompSubProcessError;
    }

    int exitCode = process.exitCode();
    if (exitCode != 0 && exitCode != 1)
        return RmeUncompSubProcessError;

    return RmeUncompSuccess;
}

void RmeUncomp7zCliPlugin::setPassword(const QString &password)
{
    m_password = password;
}

QJsonObject RmeUncomp7zCliPlugin::pluginSettings() const
{
    QJsonObject ob;
    ob[QStringLiteral("binaryPath")] = binaryPath();
    return ob;
}

bool RmeUncomp7zCliPlugin::setPluginSetting(const QString &key, const QJsonValue &value)
{
    if (key == QStringLiteral("binaryPath") && value.isString()) {
        m_binaryPath = value.toString();
        return true;
    }
    return false;
}

bool RmeUncomp7zCliPlugin::setPluginSettings(const QJsonObject &json)
{
    bool result = true;
    foreach (const QString &key, json.keys())
        result &= setPluginSetting(key, json.value(key));
    return result;
}

QString RmeUncomp7zCliPlugin::binaryPath() const
{
    if (m_binaryPath.isEmpty())
        return QStringLiteral("7zr");

    return m_binaryPath;
}

QString RmeUncomp7zCliPlugin::password() const
{
    return m_password.isEmpty() ? QStringLiteral("-") : m_password;
}
