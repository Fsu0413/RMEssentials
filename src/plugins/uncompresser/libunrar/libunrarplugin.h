#ifndef LIBUNRARPLUGIN_H
#define LIBUNRARPLUGIN_H

#include <QObject>
#include <RMEssentials/RmeUncompresser>

class QLibrary;

class RmeUncompLibUnrarPlugin
    : public QObject,
      public RmeUncompresserPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.RMEssentials.RmeUncompresserPlugin" FILE "libunrar.json")
    Q_INTERFACES(RmeUncompresserPlugin)

public:
    explicit RmeUncompLibUnrarPlugin(QObject *parent = nullptr);

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
    bool loadLibUnrar();
    QString password() const;
    QString libraryPath() const;

    QLibrary *m_libunrar;
    QString m_libraryPath;
    QString m_fileName;
    QString m_password;
};

#endif // LIBUNRARPLUGIN_H
