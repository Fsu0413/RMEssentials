#include "downloader.h"

#include <QNetworkAccessManager>
#include <QPixmap>
#include <QReadWriteLock>
#include <QThread>
#include <QTimer>

#ifdef Q_OS_OSX
#include <QStandardPaths>
#endif

// This class is to be put in the downloader thread
class DownloaderPrivate : public QObject
{
    Q_OBJECT

public:
    explicit DownloaderPrivate(Downloader *downloader);

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
    Downloader *m_downloader;

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

DownloaderPrivate::DownloaderPrivate(Downloader *downloader)
    : m_downloader(downloader)
    , m_thread(new QThread(downloader))
    , m_currentDownloadingReply(nullptr)
    , m_networkAccessManager(nullptr)
    , m_timer(nullptr)
    , m_isAll(false)
    , m_lastRecordedDownloadProgress(0u)
{
    connect(m_thread, &QThread::finished, m_downloader, &Downloader::finished);
    connect(m_thread, &QThread::finished, [this]() -> void { moveToThread(m_downloader->thread()); });
    connect(m_thread, &QThread::started, this, &DownloaderPrivate::run);
    connect(this, &DownloaderPrivate::cancel, this, &DownloaderPrivate::canceled);
}

void DownloaderPrivate::run()
{
    m_networkAccessManager = new QNetworkAccessManager(this);

    m_timer = new QTimer(this);
    m_timer->setInterval(10000);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &DownloaderPrivate::canceled);

    m_dataLock.lockForWrite();
    m_isAll = false;
    m_currentDownloadingReply = nullptr;

    QString s = Downloader::downloadPath();
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

void DownloaderPrivate::canceled()
{
    disconnect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &DownloaderPrivate::singleFileError);
    disconnect(m_currentDownloadingReply, &QNetworkReply::finished, this, &DownloaderPrivate::singleFileFinished);
    disconnect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &DownloaderPrivate::downloadProgress);

    m_currentDownloadingReply->abort();

    m_failedList << m_currentDownloadingFile;

    QTimer *timer = qobject_cast<QTimer *>(sender());
    if (timer != nullptr)
        qDebug() << m_currentDownloadingFile << "timeout";
    else
        qDebug() << m_currentDownloadingFile << "abort";

    singleFileFinished();
}

Downloader::Downloader()
    : d_ptr(new DownloaderPrivate(this))
{
    Q_D(Downloader);
    connect(this, &Downloader::destroyed, d, &DownloaderPrivate::deleteLater);
}

Downloader::~Downloader()
{
    Q_D(Downloader);
    if (d->m_thread == d->thread())
        cancel();

    if (!d->m_thread->wait(3000UL))
        d->m_thread->terminate();
}

void Downloader::start()
{
    Q_D(Downloader);
    d->moveToThread(d->m_thread);
    d->m_thread->start();
}

QString Downloader::downloadPath()
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

Downloader &Downloader::operator<<(const QString &filename)
{
    Q_D(Downloader);
    QWriteLocker wl(&d->m_dataLock);
    Q_UNUSED(wl);
    d->m_downloadSequence << filename;
    return *this;
}

const QStringList &Downloader::downloadSequence() const
{
    Q_D(const Downloader);
    QReadLocker rl(&d->m_dataLock);
    Q_UNUSED(rl);
    return d->m_downloadSequence;
}

const QStringList &Downloader::failedList() const
{
    Q_D(const Downloader);
    QReadLocker rl(&d->m_dataLock);
    Q_UNUSED(rl);
    return d->m_failedList;
}

const QString &Downloader::savePath() const
{
    Q_D(const Downloader);
    QReadLocker rl(&d->m_dataLock);
    Q_UNUSED(rl);
    return d->m_savePath;
}

void Downloader::setSavePath(const QString &sp)
{
    Q_D(Downloader);
    QWriteLocker wl(&d->m_dataLock);
    Q_UNUSED(wl);
    d->m_savePath = sp;
}

void Downloader::setIsAll(bool all)
{
    Q_D(Downloader);
    QWriteLocker wl(&d->m_dataLock);
    Q_UNUSED(wl);
    d->m_isAll = all;
}

bool Downloader::isAll() const
{
    Q_D(const Downloader);
    QReadLocker rl(&d->m_dataLock);
    Q_UNUSED(rl);
    return d->m_isAll;
}

void DownloaderPrivate::downloadSingleFile()
{
    if (m_downloadSequence.isEmpty()) {
        emit m_downloader->all_completed();
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
            emit m_downloader->one_completed(m_currentDownloadingFile);
            downloadSingleFile();
            return;
        }
    }
    m_currentDownloadingReply = m_networkAccessManager->get(QNetworkRequest(QUrl(m_currentDownloadingFile)));
    connect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &DownloaderPrivate::singleFileError);
    connect(m_currentDownloadingReply, &QNetworkReply::finished, this, &DownloaderPrivate::singleFileFinished);
    connect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &DownloaderPrivate::downloadProgress);

    m_lastRecordedDownloadProgress = 0;
    m_timer->start();
}

void DownloaderPrivate::singleFileError(QNetworkReply::NetworkError /*e*/)
{
    m_failedList << m_currentDownloadingFile;
    if (m_currentDownloadingReply != nullptr)
        qDebug() << m_currentDownloadingReply->errorString();
}

void DownloaderPrivate::downloadProgress(quint64 downloaded, quint64 total)
{
    if (downloaded - m_lastRecordedDownloadProgress > 10000) {
        m_lastRecordedDownloadProgress = downloaded;
        m_timer->start();
    }
    emit m_downloader->download_progress(downloaded, total);
}

void DownloaderPrivate::singleFileFinished()
{
    m_timer->stop();
    if (m_failedList.contains(m_currentDownloadingFile)) {
        emit m_downloader->one_failed(m_currentDownloadingFile);
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

            emit m_downloader->one_completed(m_currentDownloadingFile);
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
            connect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &DownloaderPrivate::singleFileError);
            connect(m_currentDownloadingReply, &QNetworkReply::finished, this, &DownloaderPrivate::singleFileFinished);
            connect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &DownloaderPrivate::downloadProgress);
        }
    }
}

Downloader *operator<<(Downloader *downloader, const QString &filename)
{
    (*downloader) << filename;
    return downloader;
}

void Downloader::cancel()
{
    Q_D(Downloader);
    d->m_thread->requestInterruption();
    emit d->cancel();
}

#include "downloader.moc"
