#ifndef RMEUNCOMPRESSER_H__INCLUDED
#define RMEUNCOMPRESSER_H__INCLUDED

#include "rmeglobal.h"

#include <QFlags>
#include <QStringList>
#include <QThread>

class QDir;

enum RmeUncompresserResult
{
    RmeUncompSuccess,
    RmeUncompNotReadyForUse,
    RmeUncompFileNotExist,
    RmeUncompIncorrectPassword,
    RmeUncompUnknownFileFormat,
    RmeUncompChecksumError,
    RmeUncompSubProcessError,
    RmeUncompUnknownError = -1
};

enum RmeUncompresserFormat
{
    RmeUncompZip = 0x1,
    RmeUncompRar = 0x2,
    RmeUncomp7z = 0x4
};
Q_DECLARE_FLAGS(RmeUncompresserFormats, RmeUncompresserFormat)

class LIBRMESSENTIALS_EXPORT RmeUncompresserPlugin
{
public:
    RmeUncompresserPlugin();
    virtual ~RmeUncompresserPlugin();

    virtual bool isReadyForUse() const = 0;
    virtual RmeUncompresserFormats supportedFormats() const = 0;

    virtual RmeUncompresserResult openArchive(const QString &fileName) = 0;
    virtual QStringList listFiles() = 0;
    virtual RmeUncompresserResult uncompressAllFiles(const QDir &targetDir) = 0;
    virtual RmeUncompresserResult uncompressOneFile(const QDir &targetDir, const QString &fileName) = 0;
    virtual void setPassword(const QString &password) = 0;

    virtual QJsonObject pluginSettings() const = 0;
    virtual bool setPluginSetting(const QString &key, const QJsonValue &value) = 0;
    virtual bool setPluginSettings(const QJsonObject &json) = 0;

private:
    Q_DISABLE_COPY(RmeUncompresserPlugin)
};

Q_DECLARE_INTERFACE(RmeUncompresserPlugin, "org.RMEssentials.RmeUncompresserPlugin")

class RmeUncompresserPrivate;

class LIBRMESSENTIALS_EXPORT RmeUncompresser
    : public QObject
{
    Q_OBJECT

public:
    static QStringList pluginNames();
    static RmeUncompresserPlugin *plugin(const QString &pluginName);

    bool setPlugin(RmeUncompresserFormat format, const QString &pluginName);

    RmeUncompresserResult openArchive(const QString &fileName);
    QStringList listFiles();
    RmeUncompresserResult uncompressAllFiles(const QDir &targetDir);
    RmeUncompresserResult uncompressOneFile(const QDir &targetDir, const QString &fileName);
    void setPassword(const QString &password);

    QJsonObject pluginSettings() const;
    bool setPluginSetting(const QString &key, const QJsonValue &value);
    bool setPluginSettings(const QJsonObject &json);
};

//{
//    Q_OBJECT

//public:
//    explicit RmeUncompresser(QObject *parent = nullptr);
//    ~RmeUncompresser() override;

//    void addFile(const QString &zipName, const QString &fileName);

//    void run() override;

//signals:
//    void signalFileFinished(const QString &filename);

//private:
//    Q_DISABLE_COPY(RmeUncompresser)
//    Q_DECLARE_PRIVATE(RmeUncompresser)
//    RmeUncompresserPrivate *d_ptr;
//};

#endif
