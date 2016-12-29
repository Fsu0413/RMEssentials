#ifndef __DOWNLOADER_H__
#define __DOWNLOADER_H__

#include <QDir>
#include <QNetworkReply>

class QTimer;

class DownloaderPrivate;

class Downloader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList downloadSequence READ downloadSequence)
    Q_PROPERTY(QStringList failedList READ failedList)
    Q_PROPERTY(QString savePath READ savePath WRITE setSavePath)
    Q_PROPERTY(bool isAll READ isAll WRITE setIsAll)

public:
    Downloader();
    ~Downloader();

    static QString downloadPath();

    Downloader &operator<<(const QString &filename);
    const QStringList &downloadSequence() const;
    const QStringList &failedList() const;

    const QString &savePath() const;
    void setSavePath(const QString &sp);

    void setIsAll(bool all);
    bool isAll() const;

public slots:
    void start();
    void cancel();

signals:
    void all_completed();
    void canceled();
    void one_completed(const QString &url);
    void one_failed(const QString &url);
    void error();
    void download_progress(quint64, quint64);
    void finished();

private:
    Q_DECLARE_PRIVATE(Downloader)
    DownloaderPrivate *d_ptr;
};

Downloader *operator<<(Downloader *downloader, const QString &filename);

#endif
