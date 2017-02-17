#ifndef LIBUNRARPLUGIN_H
#define LIBUNRARPLUGIN_H

#include <QObject>
#include <RMEssentials/RmeUncompresser>

class QuaZip;

class RmeUncompLibQuaZipPlugin
    : public QObject,
      public RmeUncompresserPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.RMEssentials.RmeUncompresserPlugin" FILE "quazip.json")
    Q_INTERFACES(RmeUncompresserPlugin)

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
    void setPassword(const QString &password) final override;

    QJsonObject pluginSettings() const final override;
    bool setPluginSetting(const QString &key, const QJsonValue &value) final override;
    bool setPluginSettings(const QJsonObject &json) final override;

private:
    QString password() const;

    QuaZip *m_zip;

    QString m_fileName;
    QString m_password;
};

#endif // LIBUNRARPLUGIN_H
