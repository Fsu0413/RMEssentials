#ifndef RMEUNCOMPRESSER_H__INCLUDED
#define RMEUNCOMPRESSER_H__INCLUDED

#include "rmeglobal.h"

#include <QDir>
#include <QFlags>
#include <QStringList>
#include <QThread>
#include <QVersionNumber>

Q_DECLARE_METATYPE(QDir)

enum RmeUncompresserResult
{
    RmeUncompSuccess,
    RmeUncompNotReadyForUse,
    RmeUncompFileNotExist,
    RmeUncompIncorrectPassword,
    RmeUncompUnknownFileFormat,
    RmeUncompChecksumError,
    RmeUncompSubProcessError,
    RmeUncompNoAvailablePlugin,
    RmeUncompUnknownError = -1
};
Q_DECLARE_METATYPE(RmeUncompresserResult)

enum RmeUncompresserFormat
{
    RmeUncompUnknownFormat = 0x0,
    RmeUncompZip = 0x1,
    RmeUncompRar = 0x2,
    RmeUncomp7z = 0x4
};
Q_DECLARE_FLAGS(RmeUncompresserFormats, RmeUncompresserFormat)

class RmeUncompresser;
class RmeUncompresserPluginPrivate;

class LIBRMESSENTIALS_EXPORT RmeUncompresserPlugin
    : public QObject
{
    Q_OBJECT

public:
    RmeUncompresserPlugin(QObject *parent = nullptr);
    virtual ~RmeUncompresserPlugin();

    virtual bool isReadyForUse() const = 0;
    virtual RmeUncompresserFormats supportedFormats() const = 0;

    virtual RmeUncompresserResult openArchive(const QString &fileName) = 0;
    virtual QStringList listFiles() = 0;
    virtual RmeUncompresserResult uncompressAllFiles(const QDir &targetDir) = 0;
    virtual RmeUncompresserResult uncompressOneFile(const QDir &targetDir, const QString &fileName) = 0;

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    virtual QVersionNumber versionNumber() const = 0;
#endif
    virtual const char *version() const = 0;

    QStringList propertyList() const;
    void setUncompresser(RmeUncompresser *uncompresser);

    QString password() const;

private:
    Q_DISABLE_COPY(RmeUncompresserPlugin)
    Q_DECLARE_PRIVATE(RmeUncompresserPlugin)
    RmeUncompresserPluginPrivate *d_ptr;
};

class RmeUncompresserPrivate;

class LIBRMESSENTIALS_EXPORT RmeUncompresser
    : public QObject
{
    Q_OBJECT

public:
    RmeUncompresser();
    ~RmeUncompresser();

    static QStringList pluginNames();
    static RmeUncompresserPlugin *plugin(const QString &pluginName);

    bool setPlugin(RmeUncompresserFormat format, const QString &pluginName);

    RmeUncompresserResult openArchive(const QString &fileName);
    QStringList listFiles();
    bool uncompressAllFiles(const QDir &targetDir);
    bool uncompressOneFile(const QDir &targetDir, const QString &fileName);

    void setPassword(const QString &password);
    QString password() const;

signals:
    void uncompressOneFileCompleted(RmeUncompresserResult result);
    void uncompressAllFilesCompleted(RmeUncompresserResult result);

private:
    Q_DECLARE_PRIVATE(RmeUncompresser)
    RmeUncompresserPrivate *d_ptr;
};

#endif
