#include "DownloadDialog.h"
#include "downloader.h"
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

DownloadDialog::DownloadDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Rhythm Master Downloader"));

    QVBoxLayout *alllayout = new QVBoxLayout;

    QFormLayout *flayout = new QFormLayout;
    m_nameEdit = new QLineEdit;
    connect(this, &DownloadDialog::enable_controls, m_nameEdit, &QLineEdit::setEnabled);
    QPushButton *btn = new QPushButton(tr("Download!"));
    connect(btn, &QPushButton::clicked, this, &DownloadDialog::startDownload);
    connect(this, &DownloadDialog::enable_controls, btn, &QPushButton::setEnabled);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_nameEdit);
    layout1->addWidget(btn);
    flayout->addRow(tr("Filename:"), layout1);

    m_list = new QListWidget;
    m_list->setSortingEnabled(false);

    alllayout->addLayout(flayout);
    alllayout->addWidget(m_list);
    setLayout(alllayout);
}

void DownloadDialog::startDownload() {
    static QStringList suffixs;
    static QString prefix = "http://game.ds.qq.com/Com_SongRes/song/";
    if (suffixs.isEmpty())
        suffixs << ".mp3" << ".jpg" << "_title_ipad.jpg" << "_ipad.jpg" << "_title_140_90.jpg" << "_title_hd.jpg" // do not use .png here
                << "_4k_ez.imd" << "_4k_nm.imd" << "_4k_hd.imd"
                << "_5k_ez.imd" << "_5k_nm.imd" << "_5k_hd.imd"
                << "_6k_ez.imd" << "_6k_nm.imd" << "_6k_hd.imd"
                << "_Papa_Easy.mde" << "_Papa_Normal.mde" << "_Papa_Hard.mde";


    Downloader *downloader = new Downloader;
    QString songname = m_nameEdit->text();
    foreach (const QString &suf, suffixs)
        (*downloader) << (prefix + songname + "/" + songname + suf);

    downloader->setSavePath(songname);
    
    connect(downloader, &Downloader::finished, downloader, &Downloader::deleteLater);
    connect(downloader, &Downloader::one_completed, this, &DownloadDialog::oneCompleted);
    connect(downloader, &Downloader::one_failed, this, &DownloadDialog::oneFailed);
    connect(downloader, &Downloader::all_completed, this, &DownloadDialog::allCompleted);
    connect(downloader, &Downloader::error, this, &DownloadDialog::errorOccurred);

    emit enable_controls(false);

    downloader->start();
}

void DownloadDialog::oneCompleted(const QString &url) {
    QString filename = QUrl(url).fileName();
    appendLog(filename + tr(" download successful"));
}

void DownloadDialog::oneFailed(const QString &url) {
    QString filename = QUrl(url).fileName();
    appendLog(filename + tr(" download failed"));
}

void DownloadDialog::allCompleted() {
    appendLog(tr("All files downloaded"));
    emit enable_controls(true);
}

void DownloadDialog::errorOccurred() {
    appendLog(tr("Download failed"));
    emit enable_controls(true);
}

void DownloadDialog::appendLog(const QString &log) {
    m_list->addItem(log);
    m_list->scrollToBottom();
}