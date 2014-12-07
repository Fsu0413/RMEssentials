#include "downloader.h"

#include <QNetworkAccessManager>
#include <QFile>
#include <QDir>

namespace {
    QNetworkAccessManager mgr;
}

void Downloader::run()
{
    QDir dir(m_savePath);
    if (!dir.exists()) {
        dir.cdUp();
        if (!dir.exists() || !dir.mkdir(m_savePath)) {
            emit error();
            return;
        }
    }

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
        QFile file(QDir(m_savePath).absoluteFilePath(filename));
        file.open(QIODevice::Truncate | QIODevice::WriteOnly);
        file.write(r->readAll());
        file.close();

        if (filename.endsWith(".jpg")) { // important hack!!
            QString new_filename = filename;
            new_filename.chop(4);
            new_filename.append(".png");
            QDir(m_savePath).rename(filename, new_filename);
        }

        emit one_completed(m_currentDownloadingFile);
    }

    downloadSingleFile();
}