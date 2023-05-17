#include "download.h"
#include "main.h"

#include <RMEss/RmeDownloader>
#include <RMEss/RmeUncompresser>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QListWidget>
#include <QMessageBox>
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

QString DownloadButton::startDownloadTitle = DownloadButton::tr("Download!");
QString DownloadButton::cancelDownloadTitle = DownloadButton::tr("Cancel");

DownloadButton::DownloadButton(QWidget *parent)
    : QPushButton(startDownloadTitle, parent)
{
}

void DownloadButton::setBusy(bool b)
{
    if (b)
        setText(cancelDownloadTitle);
    else
        setText(startDownloadTitle);
}

DownloadDialog::DownloadDialog(QWidget *parent)
    : QDialog(parent)
    , m_busy(false)
    , m_exitRequested(false)
    , m_encryptedChartWarned(false)
#ifdef QT_WINEXTRAS_LIB
    , m_taskbarBtn(nullptr)
#endif
{
    setWindowTitle(tr("Rhythm Master Downloader"));

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    tabWidget->addTab(createDownloadSongTab(), tr("Song && RMPs"));
    int tabN = tabWidget->addTab(createDownloadRoleTab(), tr("Role"));
    tabWidget->setTabEnabled(tabN, false);
    tabWidget->addTab(createDownloadLegacySongTab(), tr("Legacy Song && IMDs"));

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
    DownloadButton *downloadSongBtn = new DownloadButton;
    downloadSongBtn->setFixedWidth(downloadSongBtn->fontMetrics().horizontalAdvance(DownloadButton::startDownloadTitle) * 1.7);
    connect(this, &DownloadDialog::busy, downloadSongBtn, &DownloadButton::setBusy);
    connect(downloadSongBtn, &QPushButton::clicked, this, &DownloadDialog::downloadSongClicked);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_songNameCombo);
    layout1->addWidget(downloadSongBtn);
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
    // not supported
    // connect(this, &DownloadDialog::busy, this, &DownloadDialog::updateUnofficialBackgroundStatus);
    m_downloadUnofficialBackground->setChecked(false);
    m_downloadUnofficialBackground->setEnabled(false);

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
    DownloadButton *downloadRoleBtn = new DownloadButton;
    downloadRoleBtn->setFixedWidth(downloadRoleBtn->fontMetrics().horizontalAdvance(DownloadButton::startDownloadTitle) * 1.7);
    connect(this, &DownloadDialog::busy, downloadRoleBtn, &DownloadButton::setBusy);
    connect(downloadRoleBtn, &QPushButton::clicked, this, &DownloadDialog::downloadRoleClicked);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_roleNameCombo);
    layout1->addWidget(downloadRoleBtn);
    flayout->addRow(tr("Role No:"), layout1);

    QWidget *downloadRoleTab = new QWidget;
    downloadRoleTab->setLayout(flayout);

    return downloadRoleTab;
}

QWidget *DownloadDialog::createDownloadLegacySongTab()
{
    QVBoxLayout *downloadSongLayout = new QVBoxLayout;

    QFormLayout *flayout = new QFormLayout;
    m_legacySongNameCombo = new QComboBox;
    m_legacySongNameCombo->setEditable(true);
#ifdef Q_OS_ANDROID
    m_legacySongNameCombo->setStyleSheet(QStringLiteral("QComboBox{height:%1;}").arg(QFontMetrics(m_songNameCombo->font()).height() * 1.7));
#endif
    connect(this, &DownloadDialog::busy, m_legacySongNameCombo, &QComboBox::setDisabled);
    DownloadButton *downloadLegacySongBtn = new DownloadButton;
    downloadLegacySongBtn->setFixedWidth(downloadLegacySongBtn->fontMetrics().horizontalAdvance(DownloadButton::startDownloadTitle) * 1.7);
    connect(this, &DownloadDialog::busy, downloadLegacySongBtn, &DownloadButton::setBusy);
    connect(downloadLegacySongBtn, &QPushButton::clicked, this, &DownloadDialog::downloadLegacySongClicked);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_legacySongNameCombo);
    layout1->addWidget(downloadLegacySongBtn);
    flayout->addRow(tr("Filename:"), layout1);

    QPushButton *downloadAllSongBtn = new QPushButton(tr("Download All"));
    connect(downloadAllSongBtn, &QPushButton::clicked, this, &DownloadDialog::startDownloadAllLegacySong);
    connect(this, &DownloadDialog::busy, downloadAllSongBtn, &QPushButton::setDisabled);
    QPushButton *downloadMissingSongBtn = new QPushButton(tr("Download missing"));
    connect(downloadMissingSongBtn, &QPushButton::clicked, this, &DownloadDialog::startDownloadAllMissingLegacySong);
    connect(this, &DownloadDialog::busy, downloadMissingSongBtn, &QPushButton::setDisabled);
    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(downloadAllSongBtn);
    layout2->addWidget(downloadMissingSongBtn);

    downloadSongLayout->addLayout(flayout);
    downloadSongLayout->addLayout(layout2);

    QWidget *downloadSongTab = new QWidget;
    downloadSongTab->setLayout(downloadSongLayout);

    return downloadSongTab;
}

QSet<QString> DownloadDialog::loadSongClientJson(const QString &fileName)
{
    QDir dir(RmeDownloader::binDownloadPath());
    if (!dir.exists())
        return {};

    QSet<QString> paths;

    if (dir.exists(fileName)) {
        QFile f(dir.absoluteFilePath(fileName));
        bool op = f.open(QIODevice::ReadOnly);
        if (op) {
            QByteArray arr = f.readAll();
            f.close();

            QJsonDocument doc = QJsonDocument::fromJson(arr, nullptr);
            if (doc.isArray()) {
                QJsonArray arr = doc.array();
                for (const QJsonValue &v : arr) {
                    if (v.isObject()) {
                        QJsonObject ob = v.toObject();
                        if (ob.contains(QStringLiteral("m_szPath"))) {
                            QJsonValue pathv = ob.value(QStringLiteral("m_szPath"));
                            if (pathv.isString())
                                paths.insert(pathv.toString().trimmed());
                        }
                    }
                }
            }
        }
    }

    return paths;
}

QSet<QString> DownloadDialog::loadMd5ListJson(const QString &fileName)
{
    QDir dir(RmeDownloader::binDownloadPath());
    if (!dir.exists())
        return {};

    QSet<QString> paths;

    if (dir.exists(fileName)) {
        QFile f(dir.absoluteFilePath(fileName));
        bool op = f.open(QIODevice::ReadOnly);
        if (op) {
            QByteArray arr = f.readAll();
            f.close();

            QJsonDocument doc = QJsonDocument::fromJson(arr, nullptr);
            if (doc.isArray()) {
                QJsonArray arr = doc.array();
                for (const QJsonValue &v : arr) {
                    if (v.isObject()) {
                        QJsonObject ob = v.toObject();
                        if (ob.contains(QStringLiteral("filename"))) {
                            QJsonValue filenamev = ob.value(QStringLiteral("filename"));
                            if (filenamev.isString()) {
                                QString s = filenamev.toString().trimmed();
                                // clang-format off
                                static QRegularExpression rx(QRegularExpression::anchoredPattern(QStringLiteral(R"r((.+)_[456]k_((ez)|(nm)|(hd))\.imd)r")));
                                // clang-format on
                                QRegularExpressionMatch match = rx.match(s);
                                if (match.hasMatch())
                                    paths.insert(match.capturedTexts().value(1));
                            }
                        }
                    }
                }
            }
        }
    }

    return paths;
}

QSet<QString> DownloadDialog::loadMd5ListXml(const QString &fileName)
{
    QDir dir(RmeDownloader::binDownloadPath());
    if (!dir.exists())
        return {};

    QSet<QString> paths;

    if (dir.exists(fileName)) {
        QFile f(dir.absoluteFilePath(fileName));
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        while (!f.atEnd()) {
            QString s = QString::fromUtf8(f.readLine());
            s = s.trimmed();
            static QRegularExpression rx(QRegularExpression::anchoredPattern(QStringLiteral("<([0-9a-z]+)\\.mp3\\ value=\\\"[0-9a-z]+\\\"\\ \\/>")));
            QRegularExpressionMatch match = rx.match(s);
            if (match.hasMatch())
                paths.insert(match.capturedTexts().value(1));
        }
        f.close();
    }

    return paths;
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
    setWindowTitle(tr("Rhythm Master Downloader") + QStringLiteral("  ") + currentNum());
    downloadList();
}

void DownloadDialog::downloadSongClicked()
{
    if (!m_busy) {
        warnEncryptedChart();
        startDownloadSong(One);
    } else {
        emit cancelDownload();
    }
}

void DownloadDialog::downloadRoleClicked()
{
    if (!m_busy)
        startDownloadRole();
    else
        emit cancelDownload();
}

void DownloadDialog::downloadLegacySongClicked()
{
    if (!m_busy)
        startDownloadSong(OneLegacy);
    else
        emit cancelDownload();
}

void DownloadDialog::warnEncryptedChart()
{
    if (!m_encryptedChartWarned) {
        // 2023.3.4 add message box for warning:
        // currently we are downloading encrypted chart.
        QMessageBox::information(this, tr("RMEssentials"),
                                 tr("Currently RM official provides only encrypted chart on file server.<br />"
                                    "This program currently does not support decryption currently."));

        m_encryptedChartWarned = true;
    }
}

void DownloadDialog::startDownloadAllSong()
{
    warnEncryptedChart();
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
    warnEncryptedChart();
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

void DownloadDialog::startDownloadAllLegacySong()
{
    m_legacySongNameCombo->setCurrentIndex(0);
    startDownloadSong(AllLegacy);
}

void DownloadDialog::startDownloadNextLegacy()
{
    if (m_legacySongNameCombo->currentIndex() == m_legacySongNameCombo->count() - 1) {
        allCompleted();
        return;
    }

    m_legacySongNameCombo->setCurrentIndex(m_legacySongNameCombo->currentIndex() + 1);
    startDownloadSong(AllLegacy);
}

void DownloadDialog::startDownloadAllMissingLegacySong()
{
    m_legacySongNameCombo->setCurrentIndex(0);
    while (QDir(RmeDownloader::legacySongDownloadPath() + m_legacySongNameCombo->currentText()).exists()) {
        if (m_legacySongNameCombo->currentIndex() == m_legacySongNameCombo->count() - 1) {
            allCompleted();
            return;
        }

        m_legacySongNameCombo->setCurrentIndex(m_legacySongNameCombo->currentIndex() + 1);
    }

    startDownloadSong(Mis);
}

void DownloadDialog::startDownloadNextMissingLegacy()
{
    m_legacySongNameCombo->setCurrentIndex(m_legacySongNameCombo->currentIndex() + 1);
    while (QDir(RmeDownloader::legacySongDownloadPath() + m_legacySongNameCombo->currentText()).exists()) {
        if (m_legacySongNameCombo->currentIndex() == m_legacySongNameCombo->count() - 1) {
            allCompleted();
            return;
        }

        m_legacySongNameCombo->setCurrentIndex(m_legacySongNameCombo->currentIndex() + 1);
    }

    startDownloadSong(Mis);
}

void DownloadDialog::startDownloadSong(DownloadMode mode)
{
    static QStringList suffixs
        = { QStringLiteral(".mp3"),
            QStringLiteral("_ipad.jpg"),
            QStringLiteral("_thumb.jpg"), // do not use .png here

            QStringLiteral("_4k_ez.rmp"),
            QStringLiteral("_4k_nm.rmp"),
            QStringLiteral("_4k_hd.rmp"),

            QStringLiteral("_5k_ez.rmp"),
            QStringLiteral("_5k_nm.rmp"),
            QStringLiteral("_5k_hd.rmp"),

            QStringLiteral("_6k_ez.rmp"),
            QStringLiteral("_6k_nm.rmp"),
            QStringLiteral("_6k_hd.rmp"),
#if 0
            QStringLiteral("_Papa_Easy.mde"),
            QStringLiteral("_Papa_Normal.mde"),
            QStringLiteral("_Papa_Hard.mde")
#endif
          };

    static QString prefix = QStringLiteral("http://res.ds.qq.com/Test_SongRes_V2/song/");

    static QStringList suffixsLegacy = {QStringLiteral(".mp3"),
                                        QStringLiteral(".jpg"),
                                        QStringLiteral("_title_ipad.jpg"), // do not use .png here

                                        QStringLiteral("_4k_ez.imd"),
                                        QStringLiteral("_4k_nm.imd"),
                                        QStringLiteral("_4k_hd.imd"),

                                        QStringLiteral("_5k_ez.imd"),
                                        QStringLiteral("_5k_nm.imd"),
                                        QStringLiteral("_5k_hd.imd"),

                                        QStringLiteral("_6k_ez.imd"),
                                        QStringLiteral("_6k_nm.imd"),
                                        QStringLiteral("_6k_hd.imd"),

                                        QStringLiteral("_4k_ez.imd.json"),
                                        QStringLiteral("_4k_nm.imd.json"),
                                        QStringLiteral("_4k_hd.imd.json"),

                                        QStringLiteral("_5k_ez.imd.json"),
                                        QStringLiteral("_5k_nm.imd.json"),
                                        QStringLiteral("_5k_hd.imd.json"),

                                        QStringLiteral("_6k_ez.imd.json"),
                                        QStringLiteral("_6k_nm.imd.json"),
                                        QStringLiteral("_6k_hd.imd.json"),

                                        QStringLiteral("_Papa_Easy.mde"),
                                        QStringLiteral("_Papa_Normal.mde"),
                                        QStringLiteral("_Papa_Hard.mde")};

    static QString prefixLegacy = QStringLiteral("https://rm-1301553285.file.myqcloud.com/Com_SongRes/song/");

    RmeDownloader *downloader = new RmeDownloader;

    switch (mode) {
    case One:
    case All:
    case Mis: {
        QString songname = m_songNameCombo->currentText();
        foreach (const QString &suf, suffixs)
            downloader << (prefix + songname + QStringLiteral("/") + songname + suf);
        downloader->setDownloadPath(RmeDownloader::songDownloadPath() + songname);
#if 0
        // TODO maintain it here once worked
        if (m_downloadUnofficialBackground->isChecked() && m_unOfficialBackgroundList.contains(songname)) {
            static QString unOffBgPrefix = QStringLiteral("https://rmessentials.fsu0413.me/unofficialBg/");
            static QStringList unOffBgSuffixs = {QStringLiteral(".png"), QStringLiteral("_title_ipad.png")};
            foreach (const QString &suf, unOffBgSuffixs)
                downloader << (unOffBgPrefix + songname + QStringLiteral("/") + songname + suf);
        }
#endif
        break;
    }
    case OneLegacy:
    case AllLegacy:
    case MisLegacy: {
        QString songname = m_legacySongNameCombo->currentText();
        foreach (const QString &suf, suffixsLegacy)
            downloader << (prefixLegacy + songname + QStringLiteral("/") + songname + suf);
        downloader->setDownloadPath(RmeDownloader::legacySongDownloadPath() + songname);

        break;
    }
    }

    connect(downloader, &RmeDownloader::singleFileCompleted, this, &DownloadDialog::oneCompleted);
    connect(downloader, &RmeDownloader::singleFileFailed, this, &DownloadDialog::oneFailed);
    connect(downloader, &RmeDownloader::canceled, this, &DownloadDialog::canceled);
    connect(this, &DownloadDialog::cancelDownload, downloader, &RmeDownloader::cancel);
    connect(downloader, &RmeDownloader::downloadProgress, this, &DownloadDialog::downloadProgress);

    switch (mode) {
    case One:
    case OneLegacy:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::allCompleted);
        break;
    case All:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNext);
        break;
    case AllLegacy:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNextLegacy);
        break;
    case Mis:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNextMissing);
        break;
    case MisLegacy:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNextMissingLegacy);
        break;
    }

    switch (mode) {
    case All:
    case AllLegacy:
    case Mis:
    case MisLegacy:
        downloader->setSkipExisting(true);
        break;
    default:
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
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableCom.zip"), QStringLiteral("MD5List.json"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableCom.zip"), QStringLiteral("mrock_song_client_android.json"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableCom.zip"), QStringLiteral("mrock_song_client.json"));
    // unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableCom.zip"), QStringLiteral("mrock_papasong_client.json"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableComLegacy.zip"), QStringLiteral("MD5List.json"), QStringLiteral("MD5ListLegacy.json"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableComLegacy.zip"), QStringLiteral("MD5List.xml"), QStringLiteral("MD5ListLegacy.xml"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableComLegacy.zip"), QStringLiteral("mrock_song_client_android.json"),
                 QStringLiteral("mrock_song_client_android_legacy.json"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableComLegacy.zip"), QStringLiteral("mrock_song_client.json"),
                 QStringLiteral("mrock_song_client_legacy.json"));

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
    QString TableCom = QString(QStringLiteral("http://res.ds.qq.com/Table/BetaTest_V2/%1/TableCom.zip")).arg(currentNum());
    downloader << TableCom;
    QString TableComLegacy = QStringLiteral("https://rm-1301553285.file.myqcloud.com/Table/Dev/32/TableCom.zip");
    downloader << qMakePair(TableComLegacy, QStringLiteral("TableComLegacy.zip"));
#endif

#if 0
    static const QString unoffBgList = QStringLiteral("https://rmessentials.fsu0413.me/unoffBgList.txt");
    downloader << unoffBgList;
#endif

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
    QSet<QString> paths;

    paths.unite(loadMd5ListJson(QStringLiteral("MD5List.json")))
        .unite(loadSongClientJson(QStringLiteral("mrock_song_client_android.json")))
        .unite(loadSongClientJson(QStringLiteral("mrock_song_client.json")));

#if 0
    if (dir.exists(QStringLiteral("MD5List.xml"))) {
        QFile f(dir.absoluteFilePath(QStringLiteral("MD5List.xml")));
        f.open(QIODevice::ReadOnly | QIODevice::Text);
        while (!f.atEnd()) {
            QString s = QString::fromUtf8(f.readLine());
            s = s.trimmed();
            static QRegularExpression rx(QRegularExpression::anchoredPattern(QStringLiteral("<([0-9a-z]+)\\.mp3\\ value=\\\"[0-9a-z]+\\\"\\ \\/>")));
            QRegularExpressionMatch match = rx.match(s);
            if (match.hasMatch())
                paths.insert(match.capturedTexts().value(1));
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
#endif

    QSet<QString> legacyPaths;
    legacyPaths.unite(loadMd5ListXml(QStringLiteral("MD5ListLegacy.xml")))
        .unite(loadMd5ListJson(QStringLiteral("MD5ListLegacy.json")))
        .unite(loadSongClientJson(QStringLiteral("mrock_song_client_android_legacy.json")))
        .unite(loadSongClientJson(QStringLiteral("mrock_song_client_legacy.json")));

    QStringList l1 = paths.values();
    std::sort(l1.begin(), l1.end());

    m_songNameCombo->addItems(l1);

#if 0
    foreach (int id, m_rolePadUiMap.keys())
        m_roleNameCombo->addItem(QString::number(id));
#endif

    QStringList l2 = legacyPaths.values();
    std::sort(l2.begin(), l2.end());

    m_legacySongNameCombo->addItems(l2);

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
#ifdef QT_WINEXTRAS_LIB
        m_taskbarBtn->progress()->reset();
        m_taskbarBtn->progress()->show();
        m_taskbarBtn->progress()->resume();
#endif
    } else {
        if (m_exitRequested)
            reject();
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

void DownloadDialog::updateUnofficialBackgroundStatus(bool b)
{
    if (b)
        m_downloadUnofficialBackground->setDisabled(true);
    else
        m_downloadUnofficialBackground->setDisabled(m_unOfficialBackgroundList.isEmpty());
}
