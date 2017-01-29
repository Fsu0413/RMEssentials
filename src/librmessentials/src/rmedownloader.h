#ifndef RMEDOWNLOADER_H__INCLUDED
#define RMEDOWNLOADER_H__INCLUDED

#include "rmeglobal.h"

#include <QObject>

class QStringList;
class RmeDownloaderPrivate;

class LIBRMESSENTIALS_EXPORT RmeDownloader : public QObject
{
    Q_OBJECT

public:
    RmeDownloader();
    ~RmeDownloader() override;

    static QString songDownloadPath();
    static QString binDownloadPath();

    RmeDownloader &operator<<(const QString &filename);
    QStringList downloadSequence() const;

    QString downloadPath() const;
    void setDownloadPath(const QString &sp);

    void setIsAll(bool all);
    bool isAll() const;

public slots:
    void start();
    void cancel();

signals:
    void allCompleted();
    void canceled();
    void singleFileCompleted(const QString &url);
    void singleFileFailed(const QString &url);
    void error();
    void downloadProgress(quint64 downloaded, quint64 total);

private:
    Q_DISABLE_COPY(RmeDownloader)
    Q_DECLARE_PRIVATE(RmeDownloader)
    RmeDownloaderPrivate *d_ptr;
};

LIBRMESSENTIALS_EXPORT RmeDownloader *operator<<(RmeDownloader *downloader, const QString &filename);

#endif
