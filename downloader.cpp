#include "downloader.h"

#include <QNetworkAccessManager>
#include <QFile>

namespace {
    QNetworkAccessManager mgr;
}

void Downloader::run()
{
    QDir currentDir = QDir::current();
    if (!currentDir.cd("downloader")) {
        if (!currentDir.mkdir("downloader")) {
            emit error();
            return;
        }
        currentDir.cd("downloader");
    }

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
    }

    m_currentDownloadingFile = m_downloadSequence.takeFirst();
    QNetworkReply *r = mgr.get(QNetworkRequest(QUrl(m_currentDownloadingFile)));
    connect(r, ((void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error)), this, &Downloader::singleFileError);
    connect(r, &QNetworkReply::finished, this, &Downloader::singleFileFinished);
}

void Downloader::singleFileError(QNetworkReply::NetworkError /*e*/) {
    m_failedList << m_currentDownloadingFile;
    QIODevice *r = qobject_cast<QIODevice *>(sender());
    if (r != NULL)
        qDebug() << r->errorString();
}

void Downloader::singleFileFinished() {
    if (m_failedList.contains(m_currentDownloadingFile))
        emit one_failed(m_currentDownloadingFile);
    else {
        QIODevice *r = qobject_cast<QIODevice *>(sender());
        QString filename = QUrl(m_currentDownloadingFile).fileName();
        QFile file(m_downloadDir.absoluteFilePath(filename));
        file.open(QIODevice::Truncate | QIODevice::WriteOnly);
        file.write(r->readAll());
        file.close();

        if (filename.endsWith(".jpg")) { // important hack!!
            QString new_filename = filename;
            new_filename.chop(4);
            new_filename.append(".png");
            m_downloadDir.rename(filename, new_filename);
        }

        emit one_completed(m_currentDownloadingFile);
    }

    downloadSingleFile();
}