#ifndef RMEDOWNLOADER_H__INCLUDED
#define RMEDOWNLOADER_H__INCLUDED

#include "rmeglobal.h"

#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>

#include <tuple>

class RmeDownloaderPrivate;

class LIBRMESSENTIALS_EXPORT RmeDownloader : public QObject
{
    Q_OBJECT

public:
    RmeDownloader();
    ~RmeDownloader() override;

    static QString binDownloadPath();
    static QString songDownloadPath();
    static QString legacySongDownloadPath();

    RmeDownloader &operator<<(const QString &filename);
    RmeDownloader &operator<<(const std::pair<QString, QString> &fileNames);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // since Qt 6 QPair is typedef to std::pair
    RmeDownloader &operator<<(const QPair<QString, QString> &fileNames);
#endif

    QString downloadPath() const;
    void setDownloadPath(const QString &sp);

    void setSkipExisting(bool skip);
    bool skipExisting() const;

public slots:
    void start();
    void cancel();

signals:
    void allCompleted();
    void canceled();
    void singleFileCompleted(const QString &url);
    void singleFileFailed(const QString &url);
    void downloadProgress(quint64 downloaded, quint64 total);

private:
    Q_DISABLE_COPY(RmeDownloader)
    Q_DECLARE_PRIVATE(RmeDownloader)
    RmeDownloaderPrivate *d_ptr;
};

LIBRMESSENTIALS_EXPORT RmeDownloader *operator<<(RmeDownloader *downloader, const QString &filename);
LIBRMESSENTIALS_EXPORT RmeDownloader *operator<<(RmeDownloader *downloader, const std::pair<QString, QString> &filename);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
LIBRMESSENTIALS_EXPORT RmeDownloader *operator<<(RmeDownloader *downloader, const QPair<QString, QString> &filename);
#endif

#endif
