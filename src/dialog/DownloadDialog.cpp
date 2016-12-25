#include "DownloadDialog.h"
#include "downloader.h"
#include "uncompresser.h"

#include <QListWidget>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QCloseEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QShowEvent>

#ifdef Q_OS_WIN
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

DownloadDialog::DownloadDialog(QWidget *parent)
    : QDialog(parent), m_busy(false), m_exitRequested(false)
#ifdef Q_OS_WIN
    , m_taskbarBtn(NULL)
#endif
{
    setWindowTitle(tr("Rhythm Master Downloader"));

    QVBoxLayout *alllayout = new QVBoxLayout;

    QFormLayout *flayout = new QFormLayout;
    m_nameCombo = new QComboBox;
    m_nameCombo->setEditable(true);
    connect(this, &DownloadDialog::busy, m_nameCombo, &QComboBox::setDisabled);
    m_downloadBtn = new QPushButton(tr("Download!"));
    m_downloadBtn->setMaximumWidth(120);
    connect(m_downloadBtn, &QPushButton::clicked, this, &DownloadDialog::downloadClicked);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_nameCombo);
    layout1->addWidget(m_downloadBtn);
    flayout->addRow(tr("Filename:"), layout1);

    QPushButton *downloadAllBtn = new QPushButton(tr("Download All"));
    connect(downloadAllBtn, &QPushButton::clicked, this, &DownloadDialog::startDownloadAll);
    connect(this, &DownloadDialog::busy, downloadAllBtn, &QPushButton::setDisabled);
    QPushButton *downloadMissingBtn = new QPushButton(tr("Download missing"));
    connect(downloadMissingBtn, &QPushButton::clicked, this, &DownloadDialog::startDownloadAllMissing);
    connect(this, &DownloadDialog::busy, downloadMissingBtn, &QPushButton::setDisabled);
    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(downloadAllBtn);
    layout2->addWidget(downloadMissingBtn);

    m_list = new QListWidget;
    m_list->setSortingEnabled(false);

    m_progressBar = new QProgressBar;
    m_progressBar->setMinimum(0);

    alllayout->addLayout(flayout);
    alllayout->addLayout(layout2);
    alllayout->addWidget(m_list);
    alllayout->addWidget(m_progressBar);
    setLayout(alllayout);

    connect(this, &DownloadDialog::busy, this, &DownloadDialog::setBusy);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(300000);
    connect(m_timer, &QTimer::timeout, this, &DownloadDialog::timeout);
}

void DownloadDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);

#ifdef Q_OS_WIN
    if (m_taskbarBtn == NULL) {
        m_taskbarBtn = new QWinTaskbarButton(this);
        m_taskbarBtn->setWindow(windowHandle());
        QWinTaskbarProgress *prog = m_taskbarBtn->progress();
        prog->setVisible(false);
        prog->setMinimum(0);
        prog->reset();
    }
#endif
}

void DownloadDialog::downloadClicked()
{
    if (!m_busy)
        startDownload();
    else
        emit cancel_download();
}

void DownloadDialog::startDownloadAll()
{
    m_nameCombo->setCurrentIndex(0);
    startDownload(All);
}

void DownloadDialog::startDownloadNext()
{
    if (m_nameCombo->currentIndex() == m_nameCombo->count() - 1) {
        allCompleted();
        return;
    }

    m_nameCombo->setCurrentIndex(m_nameCombo->currentIndex() + 1);
    startDownload(All);
}

void DownloadDialog::startDownloadAllMissing()
{
    m_nameCombo->setCurrentIndex(0);
    while (QDir(Downloader::downloadPath() + m_nameCombo->currentText()).exists()) {
        if (m_nameCombo->currentIndex() == m_nameCombo->count() - 1) {
            allCompleted();
            return;
        }

        m_nameCombo->setCurrentIndex(m_nameCombo->currentIndex() + 1);
    }

    startDownload(Mis);
}

void DownloadDialog::startDownloadNextMissing()
{
    m_nameCombo->setCurrentIndex(m_nameCombo->currentIndex() + 1);
    while (QDir(Downloader::downloadPath() + m_nameCombo->currentText()).exists()) {
        if (m_nameCombo->currentIndex() == m_nameCombo->count() - 1) {
            allCompleted();
            return;
        }

        m_nameCombo->setCurrentIndex(m_nameCombo->currentIndex() + 1);
    }

    startDownload(Mis);
}

void DownloadDialog::startDownload(DownloadMode mode)
{
    static QStringList suffixs;
    static QString prefix = QStringLiteral("http://game.ds.qq.com/Com_SongRes/song/");
    if (suffixs.isEmpty())
        suffixs
                << QStringLiteral(".mp3")
                << QStringLiteral(".jpg")
                << QStringLiteral("_title_ipad.jpg")
                << QStringLiteral("_ipad.jpg")
                << QStringLiteral("_title_140_90.jpg")  // do not use .png here

                << QStringLiteral("_4k_ez.imd")
                << QStringLiteral("_4k_nm.imd")
                << QStringLiteral("_4k_hd.imd")

                << QStringLiteral("_5k_ez.imd")
                << QStringLiteral("_5k_nm.imd")
                << QStringLiteral("_5k_hd.imd")

                << QStringLiteral("_6k_ez.imd")
                << QStringLiteral("_6k_nm.imd")
                << QStringLiteral("_6k_hd.imd")

                << QStringLiteral("_Papa_Easy.mde")
                << QStringLiteral("_Papa_Normal.mde")
                << QStringLiteral("_Papa_Hard.mde");

    Downloader *downloader = new Downloader;
    QString songname = m_nameCombo->currentText();
    foreach (const QString &suf, suffixs)
        downloader << (prefix + songname + QStringLiteral("/") + songname + suf);

    downloader->setSavePath(songname);

    connect(downloader, &Downloader::finished, downloader, &Downloader::deleteLater);
    connect(downloader, &Downloader::one_completed, this, &DownloadDialog::oneCompleted);
    connect(downloader, &Downloader::one_failed, this, &DownloadDialog::oneFailed);
    connect(downloader, &Downloader::canceled, this, &DownloadDialog::canceled);
    connect(downloader, &Downloader::error, this, &DownloadDialog::errorOccurred);
    connect(this, &DownloadDialog::timeout, downloader, &Downloader::timeout);
    connect(this, &DownloadDialog::cancel_download, downloader, &Downloader::cancel);
    connect(downloader, &Downloader::download_progress, this, &DownloadDialog::downloadProgress);

    switch (mode) {
    case All:
        connect(downloader, &Downloader::all_completed, this, &DownloadDialog::startDownloadNext);
        downloader->setIsAll(true);
        break;
    case One:
        connect(downloader, &Downloader::all_completed, this, &DownloadDialog::allCompleted);
        break;
    case Mis:
        connect(downloader, &Downloader::all_completed, this, &DownloadDialog::startDownloadNextMissing);
        downloader->setIsAll(true);
        break;
    }

    emit busy(true);

    m_timer->start();
    downloader->start();
}

void DownloadDialog::oneCompleted(const QString &url)
{
    QString filename = QUrl(url).fileName();
    appendLog(filename + tr(" download successful"));
    m_timer->stop();
    m_timer->start();
}

void DownloadDialog::oneFailed(const QString &url)
{
    QString filename = QUrl(url).fileName();
    appendLog(filename + tr(" download failed"));
    m_timer->stop();
    m_timer->start();
}

void DownloadDialog::allCompleted()
{
    appendLog(tr("All files downloaded"));
    m_timer->stop();
#ifdef Q_OS_WIN
    m_taskbarBtn->progress()->hide();
#endif
    emit busy(false);
}

void DownloadDialog::errorOccurred()
{
    appendLog(tr("Download failed"));
    m_timer->stop();
#ifdef Q_OS_WIN
    m_taskbarBtn->progress()->stop();
#endif
    emit busy(false);
}

void DownloadDialog::canceled()
{
    appendLog(tr("Download canceled"));
    m_timer->stop();
#ifdef Q_OS_WIN
    m_taskbarBtn->progress()->stop();
#endif
    emit busy(false);
}

void DownloadDialog::oneUncompressed(const QString &filename)
{
    appendLog(filename + tr(" has been uncompressed"));
}

void DownloadDialog::startUncompress()
{
#ifdef RME_USE_QUAZIP
    Uncompresser *unc = new Uncompresser;
    unc->zipNames
            << QStringLiteral("MD5List.zip")
            << QStringLiteral("TableComBin.zip")
            << QStringLiteral("TableComBin.zip")
            << QStringLiteral("TableComBin_IOS.zip");

    unc->fileNames
            << QStringLiteral("MD5List.xml")
            << QStringLiteral("mrock_song_client_android.bin")
            << QStringLiteral("mrock_papasong_client.bin")
            << QStringLiteral("mrock_song_client.bin");

    connect(unc, &Uncompresser::finished, this, &DownloadDialog::loadPaths);
    connect(unc, &Uncompresser::finished, unc, &Uncompresser::deleteLater);
    connect(unc, &Uncompresser::signal_file_finished, this, &DownloadDialog::oneUncompressed);

    unc->start();
#endif
}

void DownloadDialog::downloadList()
{
#ifdef RME_USE_QUAZIP
    static const QString md5 = QStringLiteral("http://game.ds.qq.com/Com_SongRes/MD5List.zip");
    static const QString bin = QStringLiteral("http://game.ds.qq.com/Com_TableCom_IOS_Bin/TableComBin.zip");


    Downloader *downloader = new Downloader;
    downloader << md5 << bin;

    downloader->setSavePath(QString());

    connect(downloader, &Downloader::finished, downloader, &Downloader::deleteLater);
    connect(downloader, &Downloader::one_completed, this, &DownloadDialog::oneCompleted);
    connect(downloader, &Downloader::one_failed, this, &DownloadDialog::oneFailed);
    connect(downloader, &Downloader::all_completed, this, &DownloadDialog::downloadAndroidList);
    connect(downloader, &Downloader::canceled, this, &DownloadDialog::canceled);
    connect(downloader, &Downloader::error, this, &DownloadDialog::errorOccurred);
    connect(this, &DownloadDialog::timeout, downloader, &Downloader::timeout);
    connect(this, &DownloadDialog::cancel_download, downloader, &Downloader::cancel);
    connect(downloader, &Downloader::download_progress, this, &DownloadDialog::downloadProgress);

    emit busy(true);

    downloader->start();
#else
#ifdef Q_OS_ANDROID
    QFile::remove(Downloader::downloadPath() + "MD5List.xml");
    QFile::remove(Downloader::downloadPath() + "mrock_song_client_android.bin");
    QFile::remove(Downloader::downloadPath() + "mrock_papasong_client.bin");
    QFile::copy(Downloader::downloadPath() + "../MD5List.xml", Downloader::downloadPath() + "MD5List.xml");
    QFile::copy(Downloader::downloadPath() + "../mrock_song_client_android.bin", Downloader::downloadPath() + "mrock_song_client_android.bin");
    QFile::copy(Downloader::downloadPath() + "../mrock_papasong_client.bin", Downloader::downloadPath() + "mrock_papasong_client.bin");
#endif
    loadPaths();
#endif
}

void DownloadDialog::downloadAndroidList()
{
#ifdef RME_USE_QUAZIP
    if (QFile::exists(Downloader::downloadPath() + QStringLiteral("TableComBin.zip")))
        QFile(Downloader::downloadPath() + QStringLiteral("TableComBin.zip")).rename(Downloader::downloadPath() + QStringLiteral("TableComBin_IOS.zip"));

    static const QString bin = QStringLiteral("http://game.ds.qq.com/Com_TableCom_Android_Bin/TableComBin.zip");

    Downloader *downloader = new Downloader;
    downloader << bin;

    downloader->setSavePath(QString());

    connect(downloader, &Downloader::finished, downloader, &Downloader::deleteLater);
    connect(downloader, &Downloader::one_completed, this, &DownloadDialog::oneCompleted);
    connect(downloader, &Downloader::one_failed, this, &DownloadDialog::oneFailed);
    connect(downloader, &Downloader::all_completed, this, &DownloadDialog::startUncompress);
    connect(downloader, &Downloader::canceled, this, &DownloadDialog::canceled);
    connect(downloader, &Downloader::error, this, &DownloadDialog::errorOccurred);
    connect(this, &DownloadDialog::timeout, downloader, &Downloader::timeout);
    connect(this, &DownloadDialog::cancel_download, downloader, &Downloader::cancel);
    connect(downloader, &Downloader::download_progress, this, &DownloadDialog::downloadProgress);

    downloader->start();
#endif
}

void DownloadDialog::appendLog(const QString &log)
{
    m_list->addItem(log);
    m_list->scrollToBottom();
}

void DownloadDialog::loadPaths()
{
    m_timer->stop();
    QDir dir(Downloader::downloadPath());
    if (!dir.exists())
        return;

    QSet<QString> paths;

    if (dir.exists(QStringLiteral("MD5List.xml"))) {
        QFile f(dir.absoluteFilePath(QStringLiteral("MD5List.xml")));
        f.open(QIODevice::ReadOnly);
        while (!f.atEnd()) {
            QString s = QString::fromUtf8(f.readLine());
            s = s.trimmed();
            QRegExp rx(QStringLiteral("<([0-9a-z]+)\\.mp3\\ value=\\\"[0-9a-z]+\\\"\\ \\/>"));
            if (rx.exactMatch(s))
                paths.insert(rx.capturedTexts()[1]);
        }
        f.close();
        appendLog(QStringLiteral("MD5List.xml") + tr(" has been loaded"));
    }

    if (dir.exists(QStringLiteral("mrock_song_client_android.bin"))) {
        QFile f(dir.absoluteFilePath(QStringLiteral("mrock_song_client_android.bin")));
        f.open(QIODevice::ReadOnly);
        f.seek(0xcel);
        while (f.pos() < f.size()) {
            QByteArray s = f.peek(0x40l);
            QString path = QString::fromUtf8(s);
            paths.insert(path);
            f.seek(f.pos() + 0x33el);
        }
        f.close();
        appendLog(QStringLiteral("mrock_song_client_android.bin") + tr(" has been loaded"));
    }

    if (dir.exists(QStringLiteral("mrock_papasong_client.bin"))) {
        QFile f(dir.absoluteFilePath(QStringLiteral("mrock_papasong_client.bin")));
        f.open(QIODevice::ReadOnly);
        f.seek(0xd0l);
        while (f.pos() < f.size()) {
            QByteArray s = f.peek(0x40l);
            QString path = QString::fromUtf8(s);
            paths.insert(path);
            f.seek(f.pos() + 0x169l);
        }
        f.close();
        appendLog(QStringLiteral("mrock_papasong_client.bin") + tr(" has been loaded"));
    }

    if (dir.exists(QStringLiteral("mrock_song_client.bin"))) {
        QFile f(dir.absoluteFilePath(QStringLiteral("mrock_song_client.bin")));
        f.open(QIODevice::ReadOnly);
        f.seek(0xcel);
        while (f.pos() < f.size()) {
            QByteArray s = f.peek(0x40l);
            QString path = QString::fromUtf8(s);
            paths.insert(path);
            f.seek(f.pos() + 0x33el);
        }
        f.close();
        appendLog(QStringLiteral("mrock_song_client.bin") + tr(" has been loaded"));
    }

    QStringList l = paths.toList();

    qSort(l);

    m_nameCombo->addItems(l);

#ifdef Q_OS_WIN
    m_taskbarBtn->progress()->hide();
#endif

    appendLog(tr("All files loaded"));

    emit busy(false);
}

void DownloadDialog::setBusy(bool b)
{
    m_busy = b;
    if (b) {
        m_downloadBtn->setText(tr("Cancel"));
#ifdef Q_OS_WIN
        m_taskbarBtn->progress()->reset();
        m_taskbarBtn->progress()->show();
#endif
    } else {
        if (m_exitRequested)
            reject();
        else {
            m_downloadBtn->setText(tr("Download!"));
            m_downloadBtn->setEnabled(true);
        }
    }
}

void DownloadDialog::closeEvent(QCloseEvent *e)
{
    if (m_busy) {
        e->ignore();
        m_exitRequested = true;
        emit cancel_download();
    } else
        QDialog::closeEvent(e);
}

void DownloadDialog::downloadProgress(quint64 downloaded, quint64 total)
{
    if (m_busy) {
        m_progressBar->setMaximum(total);
        m_progressBar->setValue(downloaded);
#ifdef Q_OS_WIN
        m_taskbarBtn->progress()->setMaximum(total);
        m_taskbarBtn->progress()->setValue(downloaded);
#endif // Q_OS_WIN
    }
}
