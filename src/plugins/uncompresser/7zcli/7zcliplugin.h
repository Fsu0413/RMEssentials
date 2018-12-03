#ifndef R7ZCLIPLUGIN_H
#define R7ZCLIPLUGIN_H

#include <QObject>
#include <RMEss/RmeUncompresser>

#ifdef QT_NO_PROCESS
#error "QT_NO_PROCESS is defined, this plugin will not build"
#endif

class RmeUncomp7zCliPlugin : public RmeUncompresserPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.RMEssentials.RmeUncompresserPlugin" FILE "7zcli.json")
    Q_PROPERTY(QString binaryPath MEMBER m_binaryPath)

public:
    explicit RmeUncomp7zCliPlugin(QObject *parent = nullptr);

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
    QString binaryPath() const;

    // setting
    QString m_binaryPath;

    // current operation
    QString m_fileName;
};

#endif // 7ZCLIPLUGIN_H
