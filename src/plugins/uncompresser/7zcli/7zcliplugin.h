#ifndef R7ZCLIPLUGIN_H
#define R7ZCLIPLUGIN_H

#include <QObject>
#include <RMEssentials/RmeUncompresser>

#ifdef QT_NO_PROCESS
#error "QT_NO_PROCESS is defined, this plugin will not build"
#endif

class RmeUncomp7zCliPlugin
    : public QObject,
      public RmeUncompresserPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.RMEssentials.RmeUncompresserPlugin" FILE "7zcli.json")
    Q_INTERFACES(RmeUncompresserPlugin)

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
    void setPassword(const QString &password) final override;

    QJsonObject pluginSettings() const final override;
    bool setPluginSetting(const QString &key, const QJsonValue &value) final override;
    bool setPluginSettings(const QJsonObject &json) final override;

private:
    QString binaryPath() const;
    QString password() const;

    // setting
    QString m_binaryPath;

    // current operation
    QString m_password;
    QString m_fileName;
};

#endif // 7ZCLIPLUGIN_H
