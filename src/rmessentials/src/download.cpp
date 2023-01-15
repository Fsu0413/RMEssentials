#include "download.h"

#include <RMEss/RmeDownloader>
#include <RMEss/RmeUncompresser>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QShowEvent>
#include <QSizePolicy>
#include <QTabWidget>
#include <QVBoxLayout>

#ifdef QT_WINEXTRAS_LIB
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

DownloadDialog::DownloadDialog(QWidget *parent)
    : QDialog(parent)
    , m_busy(false)
    , m_exitRequested(false)
#ifdef QT_WINEXTRAS_LIB
    , m_taskbarBtn(nullptr)
#endif
{
    setWindowTitle(tr("Rhythm Master Downloader"));

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    tabWidget->addTab(createDownloadSongTab(), tr("Song && IMDs"));
    tabWidget->addTab(createDownloadRoleTab(), tr("Role"));
    m_list = new QListWidget;
    m_list->setSortingEnabled(false);

    m_progressBar = new QProgressBar;
    m_progressBar->setMinimum(0);

    QVBoxLayout *alllayout = new QVBoxLayout;
    alllayout->addWidget(tabWidget);
    alllayout->addWidget(m_list);
    alllayout->addWidget(m_progressBar);

    setLayout(alllayout);

    connect(this, &DownloadDialog::busy, this, &DownloadDialog::setBusy);
}

QWidget *DownloadDialog::createDownloadSongTab()
{
    QVBoxLayout *downloadSongLayout = new QVBoxLayout;

    QFormLayout *flayout = new QFormLayout;
    m_songNameCombo = new QComboBox;
    m_songNameCombo->setEditable(true);
#ifdef Q_OS_ANDROID
    m_songNameCombo->setStyleSheet(QStringLiteral("QComboBox{height:%1;}").arg(QFontMetrics(m_songNameCombo->font()).height() * 1.7));
#endif
    connect(this, &DownloadDialog::busy, m_songNameCombo, &QComboBox::setDisabled);
    QString downloadBtnTitle = tr("Download!");
    m_downloadSongBtn = new QPushButton(downloadBtnTitle);
    m_downloadSongBtn->setFixedWidth(m_downloadSongBtn->fontMetrics().horizontalAdvance(downloadBtnTitle) * 1.7);

    connect(m_downloadSongBtn, &QPushButton::clicked, this, &DownloadDialog::downloadSongClicked);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_songNameCombo);
    layout1->addWidget(m_downloadSongBtn);
    flayout->addRow(tr("Filename:"), layout1);

    QPushButton *downloadAllSongBtn = new QPushButton(tr("Download All"));
    connect(downloadAllSongBtn, &QPushButton::clicked, this, &DownloadDialog::startDownloadAllSong);
    connect(this, &DownloadDialog::busy, downloadAllSongBtn, &QPushButton::setDisabled);
    QPushButton *downloadMissingSongBtn = new QPushButton(tr("Download missing"));
    connect(downloadMissingSongBtn, &QPushButton::clicked, this, &DownloadDialog::startDownloadAllMissingSong);
    connect(this, &DownloadDialog::busy, downloadMissingSongBtn, &QPushButton::setDisabled);
    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(downloadAllSongBtn);
    layout2->addWidget(downloadMissingSongBtn);
    m_downloadUnofficialBackground = new QCheckBox(tr("Download Unofficial Background"));
    connect(this, &DownloadDialog::busy, this, &DownloadDialog::updateUnofficialBackgroundStatus);
    downloadSongLayout->addLayout(flayout);
    downloadSongLayout->addLayout(layout2);
    downloadSongLayout->addWidget(m_downloadUnofficialBackground);

    QWidget *downloadSongTab = new QWidget;
    downloadSongTab->setLayout(downloadSongLayout);

    return downloadSongTab;
}

QWidget *DownloadDialog::createDownloadRoleTab()
{
    QFormLayout *flayout = new QFormLayout;
    m_roleNameCombo = new QComboBox;
    m_roleNameCombo->setEditable(true);
#ifdef Q_OS_ANDROID
    m_roleNameCombo->setStyleSheet(QStringLiteral("QComboBox{height:%1;}").arg(QFontMetrics(m_roleNameCombo->font()).height() * 1.7));
#endif
    connect(this, &DownloadDialog::busy, m_roleNameCombo, &QComboBox::setDisabled);
    QString downloadBtnTitle = tr("Download!");
    m_downloadRoleBtn = new QPushButton(downloadBtnTitle);
    m_downloadRoleBtn->setFixedWidth(m_downloadRoleBtn->fontMetrics().horizontalAdvance(downloadBtnTitle) * 1.7);
    connect(m_downloadRoleBtn, &QPushButton::clicked, this, &DownloadDialog::downloadRoleClicked);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_roleNameCombo);
    layout1->addWidget(m_downloadRoleBtn);
    flayout->addRow(tr("Role No:"), layout1);

    QWidget *downloadRoleTab = new QWidget;
    downloadRoleTab->setLayout(flayout);

    return downloadRoleTab;
}

void DownloadDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);

#ifdef QT_WINEXTRAS_LIB
    if (m_taskbarBtn == nullptr) {
        m_taskbarBtn = new QWinTaskbarButton(this);
        m_taskbarBtn->setWindow(windowHandle());
        QWinTaskbarProgress *prog = m_taskbarBtn->progress();
        prog->setVisible(false);
        prog->setMinimum(0);
        prog->reset();
    }
#endif
    downloadList();
}

void DownloadDialog::downloadSongClicked()
{
    if (!m_busy)
        startDownloadSong();
    else
        emit cancelDownload();
}

void DownloadDialog::downloadRoleClicked()
{
    if (!m_busy)
        startDownloadRole();
    else
        emit cancelDownload();
}

void DownloadDialog::startDownloadAllSong()
{
    m_songNameCombo->setCurrentIndex(0);
    startDownloadSong(All);
}

void DownloadDialog::startDownloadNext()
{
    if (m_songNameCombo->currentIndex() == m_songNameCombo->count() - 1) {
        allCompleted();
        return;
    }

    m_songNameCombo->setCurrentIndex(m_songNameCombo->currentIndex() + 1);
    startDownloadSong(All);
}

void DownloadDialog::startDownloadAllMissingSong()
{
    m_songNameCombo->setCurrentIndex(0);
    while (QDir(RmeDownloader::songDownloadPath() + m_songNameCombo->currentText()).exists()) {
        if (m_songNameCombo->currentIndex() == m_songNameCombo->count() - 1) {
            allCompleted();
            return;
        }

        m_songNameCombo->setCurrentIndex(m_songNameCombo->currentIndex() + 1);
    }

    startDownloadSong(Mis);
}

void DownloadDialog::startDownloadNextMissing()
{
    m_songNameCombo->setCurrentIndex(m_songNameCombo->currentIndex() + 1);
    while (QDir(RmeDownloader::songDownloadPath() + m_songNameCombo->currentText()).exists()) {
        if (m_songNameCombo->currentIndex() == m_songNameCombo->count() - 1) {
            allCompleted();
            return;
        }

        m_songNameCombo->setCurrentIndex(m_songNameCombo->currentIndex() + 1);
    }

    startDownloadSong(Mis);
}

void DownloadDialog::startDownloadSong(DownloadMode mode)
{
    static QStringList suffixs = {QStringLiteral(".mp3"),
                                  QStringLiteral("_ipad.jpg"),
                                  QStringLiteral("_title_140_90.jpg"), // do not use .png here
                                  QStringLiteral("_4k_ez.imd"),
                                  QStringLiteral("_4k_nm.imd"),
                                  QStringLiteral("_4k_hd.imd"),

                                  QStringLiteral("_5k_ez.imd"),
                                  QStringLiteral("_5k_nm.imd"),
                                  QStringLiteral("_5k_hd.imd"),

                                  QStringLiteral("_6k_ez.imd"),
                                  QStringLiteral("_6k_nm.imd"),
                                  QStringLiteral("_6k_hd.imd"),

                                  QStringLiteral("_Papa_Easy.mde"),
                                  QStringLiteral("_Papa_Normal.mde"),
                                  QStringLiteral("_Papa_Hard.mde")};
    static QString prefix = QStringLiteral("http://game.ds.qq.com/Com_SongRes/song/");

    RmeDownloader *downloader = new RmeDownloader;
    QString songname = m_songNameCombo->currentText();
    foreach (const QString &suf, suffixs)
        downloader << (prefix + songname + QStringLiteral("/") + songname + suf);

    if (m_downloadUnofficialBackground->isChecked() && m_unOfficialBackgroundList.contains(songname)) {
        static QString unOffBgPrefix = QStringLiteral("https://fsu0413.github.io/RMEssentials/unofficialBg/");
        static QStringList unOffBgSuffixs = {QStringLiteral(".png"), QStringLiteral("_title_ipad.png")};
        foreach (const QString &suf, unOffBgSuffixs)
            downloader << (unOffBgPrefix + songname + QStringLiteral("/") + songname + suf);
    } else {
        static QStringList offBgSuffixs = {QStringLiteral(".jpg"), QStringLiteral("_title_ipad.jpg")}; // do not use .png here
        foreach (const QString &suf, offBgSuffixs)
            downloader << (prefix + songname + QStringLiteral("/") + songname + suf);
    }

    downloader->setDownloadPath(RmeDownloader::songDownloadPath() + songname);

    connect(downloader, &RmeDownloader::singleFileCompleted, this, &DownloadDialog::oneCompleted);
    connect(downloader, &RmeDownloader::singleFileFailed, this, &DownloadDialog::oneFailed);
    connect(downloader, &RmeDownloader::canceled, this, &DownloadDialog::canceled);
    connect(this, &DownloadDialog::cancelDownload, downloader, &RmeDownloader::cancel);
    connect(downloader, &RmeDownloader::downloadProgress, this, &DownloadDialog::downloadProgress);

    switch (mode) {
    case All:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNext);
        downloader->setSkipExisting(true);
        break;
    case One:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::allCompleted);
        break;
    case Mis:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNextMissing);
        downloader->setSkipExisting(true);
        break;
    }
    connect(downloader, &RmeDownloader::allCompleted, downloader, &RmeDownloader::deleteLater);
    connect(downloader, &RmeDownloader::canceled, downloader, &RmeDownloader::deleteLater);

    emit busy(true);

    downloader->start();
}

void DownloadDialog::startDownloadRole()
{
    static QStringList suffixs = {QStringLiteral("_hd.png"), QStringLiteral("_head.png"), QStringLiteral("_cry.png")};
    static QString prefix = QStringLiteral("http://game.ds.qq.com/Com_SongRes/icon/role/");

    RmeDownloader *downloader = new RmeDownloader;
    QString roleNum = m_roleNameCombo->currentText();
    foreach (const QString &suf, suffixs)
        downloader << (prefix + roleNum + suf);

    downloader->setDownloadPath(RmeDownloader::roleDownloadPath());

    connect(downloader, &RmeDownloader::singleFileCompleted, this, &DownloadDialog::oneCompleted);
    connect(downloader, &RmeDownloader::singleFileFailed, this, &DownloadDialog::oneFailed);
    connect(downloader, &RmeDownloader::canceled, this, &DownloadDialog::canceled);
    connect(this, &DownloadDialog::cancelDownload, downloader, &RmeDownloader::cancel);
    connect(downloader, &RmeDownloader::downloadProgress, this, &DownloadDialog::downloadProgress);
    connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNoteImage);
    connect(downloader, &RmeDownloader::allCompleted, downloader, &RmeDownloader::deleteLater);
    connect(downloader, &RmeDownloader::canceled, downloader, &RmeDownloader::deleteLater);

    emit busy(true);

    downloader->start();
}

void DownloadDialog::startDownloadNoteImage()
{
    static QString suffix = QStringLiteral(".png");
    static QString prefix = QStringLiteral("http://game.ds.qq.com/Com_SongRes/NoteImage/");

    QString roleNum = m_roleNameCombo->currentText();
    int id = roleNum.toInt();
    QStringList l = m_rolePadUiMap[id];
    bool download = false;
    foreach (const QString &s, l) {
        if (!s.isEmpty()) {
            download = true;
            break;
        }
    }
    if (!download)
        allCompleted();
    else {
        RmeDownloader *downloader = new RmeDownloader;
        foreach (const QString &s, l)
            downloader << (prefix + s + suffix);

        downloader->setDownloadPath(RmeDownloader::noteImageDownloadPath());

        connect(downloader, &RmeDownloader::singleFileCompleted, this, &DownloadDialog::oneCompleted);
        connect(downloader, &RmeDownloader::singleFileFailed, this, &DownloadDialog::oneFailed);
        connect(downloader, &RmeDownloader::canceled, this, &DownloadDialog::canceled);
        connect(this, &DownloadDialog::cancelDownload, downloader, &RmeDownloader::cancel);
        connect(downloader, &RmeDownloader::downloadProgress, this, &DownloadDialog::downloadProgress);
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::allCompleted);
        connect(downloader, &RmeDownloader::allCompleted, downloader, &RmeDownloader::deleteLater);
        connect(downloader, &RmeDownloader::canceled, downloader, &RmeDownloader::deleteLater);

        downloader->start();
    }
}

void DownloadDialog::oneCompleted(const QString &url)
{
    QString filename = QUrl(url).fileName();
    appendLog(filename + tr(" download successful"));
}

void DownloadDialog::oneFailed(const QString &url)
{
    QString filename = QUrl(url).fileName();
    appendLog(filename + tr(" download failed"));
}

void DownloadDialog::allCompleted()
{
    appendLog(tr("All files downloaded"));
#ifdef QT_WINEXTRAS_LIB
    m_taskbarBtn->progress()->hide();
#endif
    emit busy(false);
}

void DownloadDialog::canceled()
{
    appendLog(tr("Download canceled"));
#ifdef QT_WINEXTRAS_LIB
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
    RmeUncompresser *unc = new RmeUncompresser;
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("MD5List.zip"), QStringLiteral("MD5List.xml"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableComBin.zip"), QStringLiteral("mrock_song_client_android.bin"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableComBin.zip"), QStringLiteral("mrock_papasong_client.bin"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableComBin.zip"), QStringLiteral("mrock.character_client.bin"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableComBin_IOS.zip"), QStringLiteral("mrock_song_client.bin"));

    connect(unc, &RmeUncompresser::finished, this, &DownloadDialog::loadPaths);
    connect(unc, &RmeUncompresser::finished, unc, &RmeUncompresser::deleteLater);
    connect(unc, &RmeUncompresser::signalFileFinished, this, &DownloadDialog::oneUncompressed);

    unc->start();
#else
    loadPaths();
#endif
}

void DownloadDialog::downloadList()
{
    RmeDownloader *downloader = new RmeDownloader;
    downloader->setDownloadPath(RmeDownloader::binDownloadPath());
#ifdef RME_USE_QUAZIP
    static const QString md5 = QStringLiteral("http://game.ds.qq.com/Com_SongRes/MD5List.zip");
    static const QString bin = QStringLiteral("http://game.ds.qq.com/Com_TableCom_IOS_Bin/TableComBin.zip");
    static const QString andbin = QStringLiteral("http://game.ds.qq.com/Com_TableCom_Android_Bin/TableComBin.zip");

    downloader << md5 << QPair<QString, QString>(bin, QStringLiteral("TableComBin_IOS.zip")) << andbin;
#endif

    static const QString unoffBgList = QStringLiteral("https://fsu0413.github.io/RMEssentials/unoffBgList.txt");

    downloader << unoffBgList;

    connect(downloader, &RmeDownloader::singleFileCompleted, this, &DownloadDialog::oneCompleted);
    connect(downloader, &RmeDownloader::singleFileFailed, this, &DownloadDialog::oneFailed);
    connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startUncompress);
    connect(downloader, &RmeDownloader::canceled, this, &DownloadDialog::canceled);
    connect(this, &DownloadDialog::cancelDownload, downloader, &RmeDownloader::cancel);
    connect(downloader, &RmeDownloader::downloadProgress, this, &DownloadDialog::downloadProgress);
    connect(downloader, &RmeDownloader::allCompleted, downloader, &RmeDownloader::deleteLater);
    connect(downloader, &RmeDownloader::canceled, downloader, &RmeDownloader::deleteLater);

    emit busy(true);

    downloader->start();
}

void DownloadDialog::appendLog(const QString &log)
{
    m_list->addItem(log);
    m_list->scrollToBottom();
}

void DownloadDialog::loadPaths()
{
    QDir dir(RmeDownloader::binDownloadPath());
    if (!dir.exists())
        return;

    QSet<QString> paths;

    if (dir.exists(QStringLiteral("MD5List.xml"))) {
        QFile f(dir.absoluteFilePath(QStringLiteral("MD5List.xml")));
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        while (!f.atEnd()) {
            QString s = QString::fromUtf8(f.readLine());
            s = s.trimmed();
            QRegularExpression rx(QRegularExpression::anchoredPattern(QStringLiteral("<([0-9a-z]+)\\.mp3\\ value=\\\"[0-9a-z]+\\\"\\ \\/>")));
            QRegularExpressionMatch match = rx.match(s);
            if (match.hasMatch())
                paths.insert(match.capturedTexts()[1]);
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

    if (dir.exists(QStringLiteral("mrock.character_client.bin"))) {
        QFile f(dir.absoluteFilePath(QStringLiteral("mrock.character_client.bin")));
        f.open(QIODevice::ReadOnly);
        f.seek(0x30cl);
        while (f.pos() < f.size()) {
            QByteArray s = f.peek(0x2l);
            int id = *reinterpret_cast<const int16_t *>(s.constData());
            f.seek(f.pos() + 0x204l);
            s = f.peek(0x20l);
            QString padui1 = QString::fromUtf8(s);
            f.seek(f.pos() + 0x20l);
            s = f.peek(0x20l);
            QString padui2 = QString::fromUtf8(s);
            f.seek(f.pos() + 0x20l);
            s = f.peek(0x20l);
            QString padui3 = QString::fromUtf8(s);
            f.seek(f.pos() + 0x40l);
            m_rolePadUiMap[id] = QStringList {padui1, padui2, padui3};
        }
        f.close();
        appendLog(QStringLiteral("mrock.character_client.bin") + tr(" has been loaded"));
    }

    if (dir.exists(QStringLiteral("unoffBgList.txt"))) {
        QFile f(dir.absoluteFilePath(QStringLiteral("unoffBgList.txt")));
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        while (!f.atEnd()) {
            QString s = QString::fromUtf8(f.readLine());
            s = s.trimmed();
            if (!s.isEmpty())
                m_unOfficialBackgroundList << s;
        }
        f.close();
        appendLog(QStringLiteral("unoffBgList.txt") + tr(" has been loaded"));
    }

    QStringList l = paths.values();
    std::sort(l.begin(), l.end());

    m_songNameCombo->addItems(l);

    foreach (int id, m_rolePadUiMap.keys())
        m_roleNameCombo->addItem(QString::number(id));

#ifdef QT_WINEXTRAS_LIB
    m_taskbarBtn->progress()->hide();
#endif

    appendLog(tr("All files loaded"));

    emit busy(false);
}

void DownloadDialog::setBusy(bool b)
{
    m_busy = b;
    if (b) {
        m_downloadSongBtn->setText(tr("Cancel"));
        m_downloadRoleBtn->setText(tr("Cancel"));
#ifdef QT_WINEXTRAS_LIB
        m_taskbarBtn->progress()->reset();
        m_taskbarBtn->progress()->show();
        m_taskbarBtn->progress()->resume();
#endif
    } else {
        if (m_exitRequested)
            reject();
        else {
            m_downloadSongBtn->setText(tr("Download!"));
            m_downloadRoleBtn->setText(tr("Download!"));
        }
    }
}

void DownloadDialog::closeEvent(QCloseEvent *e)
{
    if (m_busy) {
        e->ignore();
        m_exitRequested = true;
        emit cancelDownload();
    } else
        QDialog::closeEvent(e);
}

void DownloadDialog::downloadProgress(quint64 downloaded, quint64 total)
{
    if (m_busy) {
        m_progressBar->setMaximum(total);
        m_progressBar->setValue(downloaded);
#ifdef QT_WINEXTRAS_LIB
        m_taskbarBtn->progress()->setMaximum(total);
        m_taskbarBtn->progress()->setValue(downloaded);
#endif // Q_OS_WIN
    }
}

void DownloadDialog::updateUnofficialBackgroundStatus(bool busy)
{
    if (busy)
        m_downloadUnofficialBackground->setDisabled(true);
    else
        m_downloadUnofficialBackground->setDisabled(m_unOfficialBackgroundList.isEmpty());
}
