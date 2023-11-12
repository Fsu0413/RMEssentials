#include "rmedownloader.h"

#include <QDebug>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#ifdef LIBRMESS_JPG2PNGHACK
#include <QPixmap>
#endif

#ifdef Q_OS_MACOS
#include <QStandardPaths>
#endif

class RmeDownloaderPrivate : public QObject
{
    Q_OBJECT

public:
    explicit RmeDownloaderPrivate(RmeDownloader *downloader);

private:
    void downloadSingleFile();

public slots:
    void singleFileFinished();
    void singleFileError(QNetworkReply::NetworkError e);
    void downloadProgress(quint64 downloaded, quint64 total);

    void run();
    void cancel();

public:
    // q-ptr
    RmeDownloader *m_downloader;

    // public to RmeDownloader only since this class is always an incomplete type outside rmedownloader.cpp
    QList<std::pair<QString, QString> > m_downloadSequence;
    QString m_downloadPath;
    bool m_skipExisting;
    bool m_canceled;
    bool m_running;

private:
    // private to RmeDownloaderPrivate only.
    QStringList m_failedList;
    std::pair<QString, QString> m_currentDownloadingFile;
    QDir m_downloadDir;
    QNetworkReply *m_currentDownloadingReply;
    QNetworkAccessManager *m_networkAccessManager;
    QTimer *m_timer;

    quint64 m_lastRecordedDownloadProgress;
};

RmeDownloaderPrivate::RmeDownloaderPrivate(RmeDownloader *downloader)
    : QObject(downloader)
    , m_downloader(downloader)
    , m_skipExisting(false)
    , m_canceled(false)
    , m_running(false)
    , m_currentDownloadingReply(nullptr)
    , m_networkAccessManager(nullptr)
    , m_timer(nullptr)
    , m_lastRecordedDownloadProgress(0u)
{
}

void RmeDownloaderPrivate::run()
{
    m_networkAccessManager = new QNetworkAccessManager(this);

    m_timer = new QTimer(this);
    m_timer->setInterval(10000);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &RmeDownloaderPrivate::cancel);

    m_currentDownloadingReply = nullptr;

    QDir dir;
    m_running = true;
    m_canceled = false;
    QString savePath = m_downloadPath;

    if (!savePath.isEmpty()) {
        if (!dir.cd(savePath)) {
            if (!dir.mkpath(savePath)) {
                //emit error();
                return;
            }
            dir.cd(savePath);
        }
    }
    m_downloadDir = dir;

    downloadSingleFile();
}

void RmeDownloaderPrivate::cancel()
{
    m_canceled = true;

    disconnect(m_currentDownloadingReply, &QNetworkReply::errorOccurred, this, &RmeDownloaderPrivate::singleFileError);
    disconnect(m_currentDownloadingReply, &QNetworkReply::finished, this, &RmeDownloaderPrivate::singleFileFinished);
    disconnect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &RmeDownloaderPrivate::downloadProgress);

    m_currentDownloadingReply->abort();

    m_failedList << m_currentDownloadingFile.first;

    QTimer *timer = qobject_cast<QTimer *>(sender());
    if (timer != nullptr)
        qDebug() << m_currentDownloadingFile.first << "timeout";
    else
        qDebug() << m_currentDownloadingFile.first << "abort";

    singleFileFinished();
}

RmeDownloader::RmeDownloader()
    : d_ptr(new RmeDownloaderPrivate(this))
{
}

RmeDownloader::~RmeDownloader()
{
    Q_D(RmeDownloader);
    if (d->m_running)
        cancel();
}

void RmeDownloader::start()
{
    Q_D(RmeDownloader);
    d->run();
}

QString RmeDownloader::binDownloadPath()
{
#if defined(Q_OS_MACOS)
    QDir currentDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    if (!currentDir.cd(QStringLiteral("RMEssentials"))) {
        if (!currentDir.mkdir(QStringLiteral("RMEssentials")))
            return QString();
        currentDir.cd(QStringLiteral("RMEssentials"));
    }
#elif defined(Q_OS_ANDROID)
    QDir currentDir(QStringLiteral("/sdcard/RM/res"));
    if (!currentDir.exists()) {
        currentDir = QDir(QStringLiteral("/sdcard"));
        if (!currentDir.mkpath(QStringLiteral("/sdcard/RM/res"))) {
            qDebug() << "can't use /sdcard/RM/res for path";
            return QString();
        }
        currentDir = QDir(QStringLiteral("/sdcard/RM/res"));
    }
#else
    QDir currentDir = QDir::current();
    if (!currentDir.cd(QStringLiteral("downloader"))) {
        if (!currentDir.mkdir(QStringLiteral("downloader")))
            return QString();
        currentDir.cd(QStringLiteral("downloader"));
    }
#endif

    QString r = currentDir.absolutePath();
    if (!r.endsWith(QStringLiteral("/")))
        r.append(QStringLiteral("/"));

    return r;
}

QString RmeDownloader::songDownloadPath()
{
    QDir currentDir(binDownloadPath());

    if (!currentDir.cd(QStringLiteral("song"))) {
        if (!currentDir.mkdir(QStringLiteral("song")))
            return QString();
        currentDir.cd(QStringLiteral("song"));
    }

    QString r = currentDir.absolutePath();
    if (!r.endsWith(QStringLiteral("/")))
        r.append(QStringLiteral("/"));

    return r;
}

QString RmeDownloader::legacySongDownloadPath()
{
    QDir currentDir(binDownloadPath());

    if (!currentDir.cd(QStringLiteral("songLegacy"))) {
        if (!currentDir.mkdir(QStringLiteral("songLegacy")))
            return QString();
        currentDir.cd(QStringLiteral("songLegacy"));
    }

    QString r = currentDir.absolutePath();
    if (!r.endsWith(QStringLiteral("/")))
        r.append(QStringLiteral("/"));

    return r;
}

RmeDownloader &RmeDownloader::operator<<(const QString &filename)
{
    Q_D(RmeDownloader);
    d->m_downloadSequence << std::make_pair(filename, QString());
    return *this;
}

RmeDownloader &RmeDownloader::operator<<(const std::pair<QString, QString> &filename)
{
    Q_D(RmeDownloader);
    d->m_downloadSequence << filename;
    return *this;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
RmeDownloader &RmeDownloader::operator<<(const QPair<QString, QString> &filename)
{
    Q_D(RmeDownloader);
    d->m_downloadSequence << std::make_pair(filename.first, filename.second);
    return *this;
}
#endif

QString RmeDownloader::downloadPath() const
{
    Q_D(const RmeDownloader);
    return d->m_downloadPath;
}

void RmeDownloader::setDownloadPath(const QString &sp)
{
    Q_D(RmeDownloader);
    d->m_downloadPath = sp;
}

void RmeDownloader::setSkipExisting(bool skip)
{
    Q_D(RmeDownloader);
    d->m_skipExisting = skip;
}

bool RmeDownloader::skipExisting() const
{
    Q_D(const RmeDownloader);
    return d->m_skipExisting;
}

void RmeDownloaderPrivate::downloadSingleFile()
{
    if (m_downloadSequence.isEmpty()) {
        m_running = false;
        emit m_downloader->allCompleted();
        return;
    } else if (m_canceled) {
        m_running = false;
        m_canceled = false;
        emit m_downloader->canceled();
        return;
    }

    m_currentDownloadingFile = m_downloadSequence.takeFirst();
    if (m_skipExisting) {
        QString filename = QUrl(m_currentDownloadingFile.first).fileName();

#ifdef LIBRMESS_JPG2PNGHACK
        if (filename.endsWith(QStringLiteral(".jpg"))) { // important hack!!
            filename.chop(4);
            filename.append(QStringLiteral(".png"));
        }
#endif

        if (m_downloadDir.exists(filename)) {
            emit m_downloader->singleFileCompleted(m_currentDownloadingFile.first);
            downloadSingleFile();
            return;
        }
    }
    m_currentDownloadingReply = m_networkAccessManager->get(QNetworkRequest(QUrl(m_currentDownloadingFile.first)));
    connect(m_currentDownloadingReply, &QNetworkReply::errorOccurred, this, &RmeDownloaderPrivate::singleFileError);
    connect(m_currentDownloadingReply, &QNetworkReply::finished, this, &RmeDownloaderPrivate::singleFileFinished);
    connect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &RmeDownloaderPrivate::downloadProgress);

    m_lastRecordedDownloadProgress = 0;
    m_timer->start();
}

void RmeDownloaderPrivate::singleFileError(QNetworkReply::NetworkError /*e*/)
{
    m_failedList << m_currentDownloadingFile.first;
    if (m_currentDownloadingReply != nullptr)
        qDebug() << m_currentDownloadingReply->errorString();
}

void RmeDownloaderPrivate::downloadProgress(quint64 downloaded, quint64 total)
{
    if (downloaded - m_lastRecordedDownloadProgress > 10000) {
        m_lastRecordedDownloadProgress = downloaded;
        m_timer->start();
    }
    emit m_downloader->downloadProgress(downloaded, total);
}

void RmeDownloaderPrivate::singleFileFinished()
{
    m_timer->stop();
    if (m_failedList.contains(m_currentDownloadingFile.first)) {
        emit m_downloader->singleFileFailed(m_currentDownloadingFile.first);
        downloadSingleFile();
    } else {
        if (m_currentDownloadingReply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull()) {
            QString filename;
            if (m_currentDownloadingFile.second.isEmpty())
                filename = QUrl(m_currentDownloadingFile.first).fileName();
            else
                filename = m_currentDownloadingFile.second;
            QFile file(m_downloadDir.absoluteFilePath(filename));
            file.open(QIODevice::Truncate | QIODevice::WriteOnly);
            file.write(m_currentDownloadingReply->readAll());
            file.close();

#ifdef LIBRMESS_JPG2PNGHACK
            if (filename.endsWith(QStringLiteral(".jpg"))) { // important hack!!
                QString new_filename = filename;
                new_filename.chop(4);
                new_filename.append(QStringLiteral(".png"));
                //m_downloadDir.rename(filename, new_filename);
                QPixmap pm;
                if (pm.load(m_downloadDir.absoluteFilePath(filename))) {
                    if (pm.save(m_downloadDir.absoluteFilePath(new_filename), "PNG")) {
                        m_downloadDir.remove(filename);
                    } else
                        qDebug() << "save png error " << new_filename;
                } else if (pm.load(m_downloadDir.absoluteFilePath(filename), "PNG")) {
                    if (pm.save(m_downloadDir.absoluteFilePath(new_filename), "PNG")) {
                        m_downloadDir.remove(filename);
                    } else
                        qDebug() << "save png error " << new_filename;
                } else
                    qDebug() << "load jpg error " << filename;
            }
#endif

            emit m_downloader->singleFileCompleted(m_currentDownloadingFile.first);
            downloadSingleFile();
        } else {
            if (m_canceled) {
                emit m_downloader->canceled();
                return;
            }
            QUrl u = m_currentDownloadingReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            if (u.isRelative())
                u = u.resolved(QUrl(m_currentDownloadingFile.first));
            qDebug() << "redirect!!" << u;
            m_currentDownloadingFile.first = u.toString();
            m_currentDownloadingReply = m_networkAccessManager->get(QNetworkRequest(u));
            connect(m_currentDownloadingReply, &QNetworkReply::errorOccurred, this, &RmeDownloaderPrivate::singleFileError);
            connect(m_currentDownloadingReply, &QNetworkReply::finished, this, &RmeDownloaderPrivate::singleFileFinished);
            connect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &RmeDownloaderPrivate::downloadProgress);
        }
    }
}

void RmeDownloader::cancel()
{
    Q_D(RmeDownloader);
    d->cancel();
}

RmeDownloader *operator<<(RmeDownloader *downloader, const QString &filename)
{
    (*downloader) << filename;
    return downloader;
}

RmeDownloader *operator<<(RmeDownloader *downloader, const std::pair<QString, QString> &filename)
{
    (*downloader) << filename;
    return downloader;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
RmeDownloader *operator<<(RmeDownloader *downloader, const QPair<QString, QString> &filename)
{
    (*downloader) << filename;
    return downloader;
}
#endif

#include "rmedownloader.moc"
