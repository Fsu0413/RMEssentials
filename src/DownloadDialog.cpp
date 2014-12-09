#include "DownloadDialog.h"
#include "downloader.h"
#include <QListWidget>
//#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "uncompresser.h"

DownloadDialog::DownloadDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Rhythm Master Downloader"));

    QVBoxLayout *alllayout = new QVBoxLayout;

    QFormLayout *flayout = new QFormLayout;
    //m_nameEdit = new QLineEdit;
    //connect(this, &DownloadDialog::enable_controls, m_nameEdit, &QLineEdit::setEnabled);
    m_nameCombo = new QComboBox;
    m_nameCombo->setEditable(true);
    connect(this, &DownloadDialog::enable_controls, m_nameCombo, &QComboBox::setEnabled);
    QPushButton *btn = new QPushButton(tr("Download!"));
    btn->setMaximumWidth(150);
    connect(btn, &QPushButton::clicked, this, &DownloadDialog::startDownload);
    connect(this, &DownloadDialog::enable_controls, btn, &QPushButton::setEnabled);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_nameCombo);
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
    QString songname = m_nameCombo->currentText();
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

void DownloadDialog::oneUncompressed(const QString &filename) {
    appendLog(filename + tr(" has been uncompressed"));
}

void DownloadDialog::startUncompress() {
    Uncompresser *unc = new Uncompresser;
    unc->zipNames << "downloader/MD5List.zip" << "downloader/TableComBin.zip" << "downloader/TableComBin.zip";
    unc->fileNames << "MD5List.xml" << "mrock_song_client_android.bin" << "mrock_papasong_client.bin";

    connect(unc, &Uncompresser::finished, this, &DownloadDialog::loadPaths);
    connect(unc, &Uncompresser::finished, unc, &Uncompresser::deleteLater);
    connect(unc, &Uncompresser::signal_file_finished, this, &DownloadDialog::oneUncompressed);

    unc->start();
}

void DownloadDialog::downloadList() {
    static const QString md5 = "http://game.ds.qq.com/Com_SongRes/MD5List.zip";
    static const QString bin = "http://game.ds.qq.com/Com_TableCom_Android_Bin/TableComBin.zip";


    Downloader *downloader = new Downloader;
    (*downloader) << md5 << bin;

    downloader->setSavePath(QString());

    connect(downloader, &Downloader::finished, downloader, &Downloader::deleteLater);
    connect(downloader, &Downloader::one_completed, this, &DownloadDialog::oneCompleted);
    connect(downloader, &Downloader::one_failed, this, &DownloadDialog::oneFailed);
    connect(downloader, &Downloader::all_completed, this, &DownloadDialog::startUncompress);
    connect(downloader, &Downloader::error, this, &DownloadDialog::errorOccurred);

    emit enable_controls(false);

    downloader->start();
}

void DownloadDialog::appendLog(const QString &log) {
    m_list->addItem(log);
    m_list->scrollToBottom();
}

void DownloadDialog::loadPaths() {
    QDir dir("downloader");
    if (!dir.exists())
        return;

    QSet<QString> paths;

    if (dir.exists("MD5List.xml")) {
        QFile f("downloader/MD5List.xml");
        f.open(QIODevice::ReadOnly);
        while (!f.atEnd()) {
            QString s = f.readLine();
            s = s.trimmed();
            QRegExp rx("<([0-9a-z]+)\\.mp3\\ value=\\\"[0-9a-z]+\\\"\\ \\/>");
            if (rx.exactMatch(s))
                paths.insert(rx.capturedTexts()[1]);
        }
        f.close();
        appendLog("MD5List.xml" + tr(" has been loaded"));
    }

    if (dir.exists("mrock_song_client_android.bin")) {
        QFile f("downloader/mrock_song_client_android.bin");
        f.open(QIODevice::ReadOnly);
        f.seek(0xcel);
        while (f.pos() < f.size()) {
            qDebug() << f.pos();
            QByteArray s = f.peek(0x40l);
            QString path = QString::fromUtf8(s);
            paths.insert(path);
            f.seek(f.pos() + 0x33el);
        }
        f.close();
        appendLog("mrock_song_client_android.bin" + tr(" has been loaded"));
    }

    if (dir.exists("mrock_papasong_client.bin")) {
        QFile f("downloader/mrock_papasong_client.bin");
        f.open(QIODevice::ReadOnly);
        f.seek(0xd0l);
        while (f.pos() < f.size()) {
            qDebug() << f.pos();
            QByteArray s = f.peek(0x40l);
            QString path = QString::fromUtf8(s);
            paths.insert(path);
            f.seek(f.pos() + 0x169l);
        }
        f.close();
        appendLog("mrock_papasong_client.bin" + tr(" has been loaded"));
    }

    QStringList l;
    foreach (const QString &path, paths)
        l << path;

    qSort(l);

    m_nameCombo->addItems(l);

    appendLog(tr("All files loaded"));

    emit enable_controls(true);
}
