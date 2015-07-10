#include "downloader.h"

#include <QNetworkAccessManager>
#include <QFile>
#include <QPixmap>

namespace {
    QNetworkAccessManager mgr;
}

Downloader::Downloader() {
    m_isAll = false;
    m_currentDownloadingReply = NULL;
}

void Downloader::run()
{
    m_cancelRequested = false;
#ifndef Q_OS_ANDROID
    QDir currentDir = QDir::current();
    if (!currentDir.cd("downloader")) {
        if (!currentDir.mkdir("downloader")) {
            emit error();
            return;
        }
        currentDir.cd("downloader");
    }
#else
    QDir currentDir( "/sdcard/RM/res/song");
#endif

    QDir dir = currentDir;
    if (!m_savePath.isEmpty()) {
        if (!dir.cd(m_savePath)) {
            if (!dir.mkdir(m_savePath)) {
                emit error();
                return;
            }
            dir.cd(m_savePath);
        }
    }

    m_downloadDir = dir;

    downloadSingleFile();
    
    exec();
}

void Downloader::downloadSingleFile() {
    if (m_downloadSequence.isEmpty()) {
        emit all_completed();
        quit();
        return;
    } else if (m_cancelRequested) {
        emit canceled();
        quit();
        return;
    }

    m_currentDownloadingFile = m_downloadSequence.takeFirst();
    if (m_isAll) {
        QString filename = QUrl(m_currentDownloadingFile).fileName();
        if (filename.endsWith(".jpg")) { // important hack!!
            filename.chop(4);
            filename.append(".png");
        }

        if (m_downloadDir.exists(filename)) {
            emit one_completed(m_currentDownloadingFile);
            downloadSingleFile();
            return;
        }
    }
    m_currentDownloadingReply = mgr.get(QNetworkRequest(QUrl(m_currentDownloadingFile)));
    connect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &Downloader::singleFileError);
    connect(m_currentDownloadingReply, &QNetworkReply::finished, this, &Downloader::singleFileFinished);
    connect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &Downloader::download_progress);
}

void Downloader::singleFileError(QNetworkReply::NetworkError /*e*/) {
    m_failedList << m_currentDownloadingFile;
    if (m_currentDownloadingReply != NULL)
        qDebug() << m_currentDownloadingReply->errorString();
}

void Downloader::singleFileFinished() {
    if (m_failedList.contains(m_currentDownloadingFile)) {
        emit one_failed(m_currentDownloadingFile);
        downloadSingleFile();
    } else {
        if (m_currentDownloadingReply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull()) {
            QString filename = QUrl(m_currentDownloadingFile).fileName();
            QFile file(m_downloadDir.absoluteFilePath(filename));
            file.open(QIODevice::Truncate | QIODevice::WriteOnly);
            file.write(m_currentDownloadingReply->readAll());
            file.close();

            if (filename.endsWith(".jpg")) { // important hack!!
                QString new_filename = filename;
                new_filename.chop(4);
                new_filename.append(".png");
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

            emit one_completed(m_currentDownloadingFile);
            downloadSingleFile();
        } else {
            if (m_cancelRequested) {
                emit canceled();
                quit();
                return;
            }
            QUrl u = m_currentDownloadingReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            if (u.isRelative())
                u = u.resolved(QUrl(m_currentDownloadingFile));
            qDebug() << "redirect!!";
            qDebug() << u;
            m_currentDownloadingFile = u.toString();
            m_currentDownloadingReply = mgr.get(QNetworkRequest(u));
            connect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &Downloader::singleFileError);
            connect(m_currentDownloadingReply, &QNetworkReply::finished, this, &Downloader::singleFileFinished);
            connect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &Downloader::download_progress);
        }
    }
}

Downloader *operator <<(Downloader *downloader, const QString &filename) {
    (*downloader) << filename;
    return downloader;
}

void Downloader::cancel() {
    m_currentDownloadingReply->abort();

    disconnect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &Downloader::singleFileError);
    disconnect(m_currentDownloadingReply, &QNetworkReply::finished, this, &Downloader::singleFileFinished);
    disconnect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &Downloader::download_progress);

    m_failedList << m_currentDownloadingFile;
    qDebug() << m_currentDownloadingFile << "abort";

    m_cancelRequested = true;

    singleFileFinished();
}

void Downloader::timeout() {
    m_currentDownloadingReply->abort();

    disconnect(m_currentDownloadingReply, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &Downloader::singleFileError);
    disconnect(m_currentDownloadingReply, &QNetworkReply::finished, this, &Downloader::singleFileFinished);
    disconnect(m_currentDownloadingReply, &QNetworkReply::downloadProgress, this, &Downloader::download_progress);

    m_failedList << m_currentDownloadingFile;
    qDebug() << m_currentDownloadingFile << "timeout";

    singleFileFinished();
}
