#include "rmedownloader.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QReadWriteLock>
#include <QThread>
#include <QTimer>

#ifdef Q_OS_OSX
#include <QStandardPaths>
#endif

// This class is to be put in the downloader thread
class RmeDownloaderPrivate : public QObject
{
    Q_OBJECT

public:
    explicit RmeDownloaderPrivate(RmeDownloader *downloader);

private:
    void downloadSingleFile();

private slots:
    void singleFileFinished();
    void singleFileError(QNetworkReply::NetworkError e);
    void downloadProgress(quint64 downloaded, quint64 total);

    void run();
    void canceled();

signals:
    void cancel();

public: // public to Downloader only since this is not interface of Downloader
    RmeDownloader *m_downloader;

    QThread *m_thread;
    mutable QReadWriteLock m_dataLock;

    QStringList m_downloadSequence;
    QStringList m_failedList;
    QString m_savePath;
    QString m_currentDownloadingFile;
    QDir m_downloadDir;
    QNetworkReply *m_currentDownloadingReply;
    QNetworkAccessManager *m_networkAccessManager;
    QTimer *m_timer;

    bool m_isAll;
    quint64 m_lastRecordedDownloadProgress;
};

RmeDownloaderPrivate::RmeDownloaderPrivate(RmeDownloader *downloader)
    : m_downloader(downloader)
    , m_thread(new QThread(downloader))
    , m_currentDownloadingReply(nullptr)
    , m_networkAccessManager(nullptr)
    , m_timer(nullptr)
    , m_isAll(false)
    , m_lastRecordedDownloadProgress(0u)
{
    connect(m_thread, &QThread::finished, m_downloader, &RmeDownloader::finished);
    connect(m_thread, &QThread::finished, [this]() -> void { moveToThread(m_downloader->thread()); });
    connect(m_thread, &QThread::started, this, &RmeDownloaderPrivate::run);
    connect(this, &RmeDownloaderPrivate::cancel, this, &RmeDownloaderPrivate::canceled);
}

void RmeDownloaderPrivate::run()
{
    m_networkAccessManager = new QNetworkAccessManager(this);

    m_timer = new QTimer(this);
    m_timer->setInterval(10000);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &RmeDownloaderPrivate::canceled);

    m_dataLock.lockForWrite();
    m_isAll = false;
    m_currentDownloadingReply = nullptr;

    QString s = RmeDownloader::downloadPath();
    if (s.isEmpty()) {
        // emit error();
        return;
    }

    QDir dir(s);
    if (!m_savePath.isEmpty()) {
        if (!dir.cd(m_savePath)) {
            if (!dir.mkdir(m_savePath)) {
                //emit error();
                return;
            }
            dir.cd(m_savePath);
        }
    }
    m_downloadDir = dir;
    m_dataLock.unlock();

    downloadSingleFile();
}

void RmeDownloaderPrivate::canceled()
{
    disconnect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &RmeDownloaderPrivate::singleFileError);
    disconnect(m_currentDownloadingReply, &QNetworkReply::finished, this, &RmeDownloaderPrivate::singleFileFinished);
    disconnect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &RmeDownloaderPrivate::downloadProgress);

    m_currentDownloadingReply->abort();

    m_failedList << m_currentDownloadingFile;

    QTimer *timer = qobject_cast<QTimer *>(sender());
    if (timer != nullptr)
        qDebug() << m_currentDownloadingFile << "timeout";
    else
        qDebug() << m_currentDownloadingFile << "abort";

    singleFileFinished();
}

RmeDownloader::RmeDownloader()
    : d_ptr(new RmeDownloaderPrivate(this))
{
    Q_D(RmeDownloader);
    connect(this, &RmeDownloader::destroyed, d, &RmeDownloaderPrivate::deleteLater);
}

RmeDownloader::~RmeDownloader()
{
    Q_D(RmeDownloader);
    if (d->m_thread == d->thread())
        cancel();

    if (!d->m_thread->wait(3000UL))
        d->m_thread->terminate();
}

void RmeDownloader::start()
{
    Q_D(RmeDownloader);
    d->moveToThread(d->m_thread);
    d->m_thread->start();
}

QString RmeDownloader::downloadPath()
{
#if defined(Q_OS_WIN)
    QDir currentDir = QDir::current();
    if (!currentDir.cd(QStringLiteral("downloader"))) {
        if (!currentDir.mkdir(QStringLiteral("downloader")))
            return QString();
        currentDir.cd(QStringLiteral("downloader"));
    }
#elif defined(Q_OS_OSX)
    QDir currentDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    if (!currentDir.cd("RMEssentials")) {
        if (!currentDir.mkdir("RMEssentials"))
            return QString();
        currentDir.cd("RMEssentials");
    }
#elif defined(Q_OS_ANDROID)
    QDir currentDir("/sdcard/RM/res/song");
    if (!currentDir.exists())
        return QString();
#endif

    QString r = currentDir.absolutePath();
    if (!r.endsWith(QStringLiteral("/")))
        r.append(QStringLiteral("/"));

    return r;
}

RmeDownloader &RmeDownloader::operator<<(const QString &filename)
{
    Q_D(RmeDownloader);
    QWriteLocker wl(&d->m_dataLock);
    Q_UNUSED(wl);
    d->m_downloadSequence << filename;
    return *this;
}

const QStringList &RmeDownloader::downloadSequence() const
{
    Q_D(const RmeDownloader);
    QReadLocker rl(&d->m_dataLock);
    Q_UNUSED(rl);
    return d->m_downloadSequence;
}

const QStringList &RmeDownloader::failedList() const
{
    Q_D(const RmeDownloader);
    QReadLocker rl(&d->m_dataLock);
    Q_UNUSED(rl);
    return d->m_failedList;
}

const QString &RmeDownloader::savePath() const
{
    Q_D(const RmeDownloader);
    QReadLocker rl(&d->m_dataLock);
    Q_UNUSED(rl);
    return d->m_savePath;
}

void RmeDownloader::setSavePath(const QString &sp)
{
    Q_D(RmeDownloader);
    QWriteLocker wl(&d->m_dataLock);
    Q_UNUSED(wl);
    d->m_savePath = sp;
}

void RmeDownloader::setIsAll(bool all)
{
    Q_D(RmeDownloader);
    QWriteLocker wl(&d->m_dataLock);
    Q_UNUSED(wl);
    d->m_isAll = all;
}

bool RmeDownloader::isAll() const
{
    Q_D(const RmeDownloader);
    QReadLocker rl(&d->m_dataLock);
    Q_UNUSED(rl);
    return d->m_isAll;
}

void RmeDownloaderPrivate::downloadSingleFile()
{
    if (m_downloadSequence.isEmpty()) {
        emit m_downloader->allCompleted();
        m_thread->quit();
        return;
    } else if (m_thread->isInterruptionRequested()) {
        emit m_downloader->canceled();
        m_thread->quit();
        return;
    }

    m_currentDownloadingFile = m_downloadSequence.takeFirst();
    if (m_isAll) {
        QString filename = QUrl(m_currentDownloadingFile).fileName();
        if (filename.endsWith(QStringLiteral(".jpg"))) { // important hack!!
            filename.chop(4);
            filename.append(QStringLiteral(".png"));
        }

        if (m_downloadDir.exists(filename)) {
            emit m_downloader->singleFileCompleted(m_currentDownloadingFile);
            downloadSingleFile();
            return;
        }
    }
    m_currentDownloadingReply = m_networkAccessManager->get(QNetworkRequest(QUrl(m_currentDownloadingFile)));
    connect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &RmeDownloaderPrivate::singleFileError);
    connect(m_currentDownloadingReply, &QNetworkReply::finished, this, &RmeDownloaderPrivate::singleFileFinished);
    connect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &RmeDownloaderPrivate::downloadProgress);

    m_lastRecordedDownloadProgress = 0;
    m_timer->start();
}

void RmeDownloaderPrivate::singleFileError(QNetworkReply::NetworkError /*e*/)
{
    m_failedList << m_currentDownloadingFile;
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
    if (m_failedList.contains(m_currentDownloadingFile)) {
        emit m_downloader->singleFileFailed(m_currentDownloadingFile);
        downloadSingleFile();
    } else {
        if (m_currentDownloadingReply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull()) {
            QString filename = QUrl(m_currentDownloadingFile).fileName();
            QFile file(m_downloadDir.absoluteFilePath(filename));
            file.open(QIODevice::Truncate | QIODevice::WriteOnly);
            file.write(m_currentDownloadingReply->readAll());
            file.close();

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

            emit m_downloader->singleFileCompleted(m_currentDownloadingFile);
            downloadSingleFile();
        } else {
            if (m_thread->isInterruptionRequested()) {
                emit m_downloader->canceled();
                m_thread->quit();
                return;
            }
            QUrl u = m_currentDownloadingReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            if (u.isRelative())
                u = u.resolved(QUrl(m_currentDownloadingFile));
            qDebug() << "redirect!!";
            qDebug() << u;
            m_currentDownloadingFile = u.toString();
            m_currentDownloadingReply = m_networkAccessManager->get(QNetworkRequest(u));
            connect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &RmeDownloaderPrivate::singleFileError);
            connect(m_currentDownloadingReply, &QNetworkReply::finished, this, &RmeDownloaderPrivate::singleFileFinished);
            connect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &RmeDownloaderPrivate::downloadProgress);
        }
    }
}

RmeDownloader *operator<<(RmeDownloader *downloader, const QString &filename)
{
    (*downloader) << filename;
    return downloader;
}

void RmeDownloader::cancel()
{
    Q_D(RmeDownloader);
    d->m_thread->requestInterruption();
    emit d->cancel();
}

#include "rmedownloader.moc"
