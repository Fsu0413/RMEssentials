#include "download.h"
#include "main.h"

#include <RmEss/RmeCrypt>
#include <RmEss/RmeDownloader>
#include <RmEss/RmeUncompresser>

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
#include <QString>
#include <QStringList>
#include <QTabWidget>
#include <QVBoxLayout>

#ifdef QT_WINEXTRAS_LIB
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

#include <tuple>

namespace {

const QString &startDownloadTitle()
{
    static const QString title = DownloadButton::tr("Download!");
    return title;
}
const QString &cancelDownloadTitle()
{
    static const QString title = DownloadButton::tr("Cancel");
    return title;
}

}

DownloadButton::DownloadButton(QWidget *parent)
    : QPushButton(startDownloadTitle(), parent)
{
}

void DownloadButton::setBusy(bool b)
{
    if (b)
        setText(cancelDownloadTitle());
    else
        setText(startDownloadTitle());
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
    downloadSongBtn->setFixedWidth(downloadSongBtn->fontMetrics().horizontalAdvance(startDownloadTitle()) * 1.7);
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

    downloadSongLayout->addLayout(flayout);
    downloadSongLayout->addLayout(layout2);

    QWidget *downloadSongTab = new QWidget;
    downloadSongTab->setLayout(downloadSongLayout);

    return downloadSongTab;
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
    downloadLegacySongBtn->setFixedWidth(downloadLegacySongBtn->fontMetrics().horizontalAdvance(startDownloadTitle()) * 1.7);
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

QSet<QString> DownloadDialog::loadSongClientJsonImpl(const QByteArray &arr, const QString &fileName)
{
    QSet<QString> paths;

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

    appendLog(fileName + tr(" has been loaded"));

    return paths;
}

QSet<QString> DownloadDialog::loadSongClientJson(const QString &fileName)
{
    QDir dir(RmeDownloader::binDownloadPath());
    if (!dir.exists())
        return {};

    if (dir.exists(fileName)) {
        QFile f(dir.absoluteFilePath(fileName));
        bool op = f.open(QIODevice::ReadOnly);
        if (op) {
            QByteArray arr = f.readAll();
            f.close();

            return loadSongClientJsonImpl(arr, fileName);
        }
    }

    return {};
}

QSet<QString> DownloadDialog::loadSongClientJsonEncrypted(const QString &fileName)
{
    QDir dir(RmeDownloader::binDownloadPath());
    if (!dir.exists())
        return {};

    if (dir.exists(fileName)) {
        QFile f(dir.absoluteFilePath(fileName));
        bool op = f.open(QIODevice::ReadOnly);
        if (op) {
            QByteArray arr = f.readAll();
            f.close();

            QByteArray decrypted = RmeCrypt::decryptFull(arr, currentHash());
            if (!decrypted.isEmpty()) {
                if (fileName.endsWith(QStringLiteral(".encrypted"))) {
                    QString decryptedFileName = fileName.chopped(10).append(QStringLiteral(".decrypted"));

                    QFile fd(dir.absoluteFilePath(decryptedFileName));
                    bool opd = fd.open(QIODevice::WriteOnly | QIODevice::Truncate);
                    if (opd) {
                        fd.write(decrypted);
                        fd.close();
                    }
                }

                return loadSongClientJsonImpl(decrypted, fileName);
            }
        }
    }

    return {};
}

QSet<QString> DownloadDialog::loadMd5ListJsonImpl(const QByteArray &arr, const QString &fileName)
{
    QSet<QString> paths;

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
                        {
                            // clang-format off
                            static QRegularExpression rxImd(QRegularExpression::anchoredPattern(QStringLiteral(R"r((.+)_[456]k_((ez)|(nm)|(hd))\.imd)r")));
                            // clang-format on
                            QRegularExpressionMatch match = rxImd.match(s);
                            if (match.hasMatch())
                                paths.insert(match.capturedTexts().value(1));
                        }
                        {
                            // clang-format off
                            static QRegularExpression rxMp3(QRegularExpression::anchoredPattern(QStringLiteral(R"r((.+)\.mp3)r")));
                            // clang-format on
                            QRegularExpressionMatch match = rxMp3.match(s);
                            if (match.hasMatch())
                                paths.insert(match.capturedTexts().value(1));
                        }
                    }
                }
            }
        }
    }

    appendLog(fileName + tr(" has been loaded"));

    return paths;
}

QSet<QString> DownloadDialog::loadMd5ListJson(const QString &fileName)
{
    QDir dir(RmeDownloader::binDownloadPath());
    if (!dir.exists())
        return {};

    if (dir.exists(fileName)) {
        QFile f(dir.absoluteFilePath(fileName));
        bool op = f.open(QIODevice::ReadOnly);
        if (op) {
            QByteArray arr = f.readAll();
            f.close();

            return loadMd5ListJsonImpl(arr, fileName);
        }
    }

    return {};
}

QSet<QString> DownloadDialog::loadMd5ListJsonEncrypted(const QString &fileName)
{
    QDir dir(RmeDownloader::binDownloadPath());
    if (!dir.exists())
        return {};

    if (dir.exists(fileName)) {
        QFile f(dir.absoluteFilePath(fileName));
        bool op = f.open(QIODevice::ReadOnly);
        if (op) {
            QByteArray arr = f.readAll();
            f.close();

            QByteArray decrypted = RmeCrypt::decryptFull(arr, currentHash());
            if (!decrypted.isEmpty()) {
                if (fileName.endsWith(QStringLiteral(".encrypted"))) {
                    QString decryptedFileName = fileName.chopped(10).append(QStringLiteral(".decrypted"));

                    QFile fd(dir.absoluteFilePath(decryptedFileName));
                    bool opd = fd.open(QIODevice::WriteOnly | QIODevice::Truncate);
                    if (opd) {
                        fd.write(decrypted);
                        fd.close();
                    }
                }

                return loadMd5ListJsonImpl(decrypted, fileName);
            }
        }
    }

    return {};
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

    appendLog(fileName + tr(" has been loaded"));

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
                                    "Decryption will be implemented elsewhere in this program (TBD)."));

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
    // clang-format off
    static QStringList suffixs = {
        QStringLiteral(".mp3"),
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
    };

    static QStringList suffixsLegacy = {
        QStringLiteral(".mp3"),
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
        QStringLiteral("_Papa_Hard.mde"),
    };
    // clang-format on

    static QString prefix = QStringLiteral("https://res.ds.qq.com/SongRes/song/");
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
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNext, Qt::QueuedConnection);
        break;
    case AllLegacy:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNextLegacy, Qt::QueuedConnection);
        break;
    case Mis:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNextMissing, Qt::QueuedConnection);
        break;
    case MisLegacy:
        connect(downloader, &RmeDownloader::allCompleted, this, &DownloadDialog::startDownloadNextMissingLegacy, Qt::QueuedConnection);
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
    RmeUncompresser *unc = new RmeUncompresser;
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableEnc.zip"), QStringLiteral("MD5List.json"), QStringLiteral("MD5List.json.encrypted"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableEnc.zip"), QStringLiteral("mrock_song_client_android.json"),
                 QStringLiteral("mrock_song_client_android.json.encrypted"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableEnc.zip"), QStringLiteral("mrock_song_client.json"), QStringLiteral("mrock_song_client.json.encrypted"));
    unc->addFile(RmeDownloader::binDownloadPath() + QStringLiteral("TableEnc.zip"), QStringLiteral("mrock_song_client_temp.json"),
                 QStringLiteral("mrock_song_client_temp.json.encrypted"));
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
}

void DownloadDialog::downloadList()
{
    RmeDownloader *downloader = new RmeDownloader;
    downloader->setDownloadPath(RmeDownloader::binDownloadPath());

    QString TableEnc = QString(QStringLiteral("https://res.ds.qq.com/Table/Release/%1/TableEnc.zip")).arg(currentNum());
    downloader << TableEnc;
    QString TableComLegacy = QStringLiteral("https://rm-1301553285.file.myqcloud.com/Table/Dev/32/TableCom.zip");
    downloader << std::make_pair(TableComLegacy, QStringLiteral("TableComLegacy.zip"));

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
    paths.unite(loadMd5ListJsonEncrypted(QStringLiteral("MD5List.json.encrypted")))
        .unite(loadSongClientJsonEncrypted(QStringLiteral("mrock_song_client_android.json.encrypted")))
        .unite(loadSongClientJsonEncrypted(QStringLiteral("mrock_song_client.json.encrypted")))
        .unite(loadSongClientJsonEncrypted(QStringLiteral("mrock_song_client_temp.json.encrypted")));

    QSet<QString> legacyPaths;
    legacyPaths.unite(loadMd5ListXml(QStringLiteral("MD5ListLegacy.xml")))
        .unite(loadMd5ListJson(QStringLiteral("MD5ListLegacy.json")))
        .unite(loadSongClientJson(QStringLiteral("mrock_song_client_android_legacy.json")))
        .unite(loadSongClientJson(QStringLiteral("mrock_song_client_legacy.json")));

    QStringList l1 = paths.values();
    std::sort(l1.begin(), l1.end());

    m_songNameCombo->addItems(l1);

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
