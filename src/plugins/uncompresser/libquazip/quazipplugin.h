#ifndef LIBUNRARPLUGIN_H
#define LIBUNRARPLUGIN_H

#include <QObject>
#include <RMEssentials/RmeUncompresser>

class QuaZip;

class RmeUncompLibQuaZipPlugin
    : public RmeUncompresserPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.RMEssentials.RmeUncompresserPlugin" FILE "quazip.json")

public:
    explicit RmeUncompLibQuaZipPlugin(QObject *parent = nullptr);

    // RmeUncompresserPlugin interface
public:
    bool isReadyForUse() const final override;
    RmeUncompresserFormats supportedFormats() const final override;

    RmeUncompresserResult openArchive(const QString &fileName) final override;
    QStringList listFiles() final override;
    RmeUncompresserResult uncompressAllFiles(const QDir &targetDir) final override;
    RmeUncompresserResult uncompressOneFile(const QDir &targetDir, const QString &fileName) final override;

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QVersionNumber versionNumber() const final override;
#endif
    const char *version() const final override;

private:
    QuaZip *m_zip;

    QString m_fileName;
};

#endif // LIBUNRARPLUGIN_H
